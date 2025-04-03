package auth

import (
	"database/sql"
	"html/template"
	"net/http"
	"time"

	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/golang-jwt/jwt/v5"
	"github.com/matthewhartstonge/argon2"
)

func RenderLoginTemplate(w http.ResponseWriter, r *http.Request, template *template.Template) error {
	return template.ExecuteTemplate(w, "login.html", nil)
}

func Login(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
	loginRequest := types.SignupRequest{}

	err := r.ParseForm()
	if err != nil {
		return err
	}

	loginRequest.Username = r.PostFormValue("username")
	loginRequest.Password = r.PostFormValue("password")

	tx, err := ctx.DB.Begin()
	if err != nil {
		return err
	}
	defer tx.Rollback()

	qtx := ctx.Repositories.WithTx(tx)

	user, err := qtx.GetUserByUsername(ctx.Ctx, sql.NullString{String: loginRequest.Username, Valid: true})
	if err != nil {
		if err == sql.ErrNoRows {
			http.Error(w, "incorrect username", http.StatusUnauthorized)
			return nil
		} else {
			return err
		}
	}

	ok, err := argon2.VerifyEncoded([]byte(loginRequest.Password), []byte(user.Passworddigest.String))
	if err != nil {
		return err
	}

	if !ok {
		http.Error(w, "incorrect password", http.StatusUnauthorized)
		return nil
	}

	claims := jwt.MapClaims{
		"username": user.Username,
		"user_id":  user.ID,
		"exp":      time.Now().Add(24 * time.Hour).Unix(),
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)

	signedToken, err := token.SignedString([]byte(ctx.Config.JwtSigningSecret))
	if err != nil {
		return err
	}

	http.SetCookie(w, &http.Cookie{
		Name:     "dashboard",
		Value:    signedToken,
		HttpOnly: true,
		Path:     "/",
		Domain:   ctx.Config.CookieDomain,
		Expires:  time.Now().Add(24 * time.Hour),
	})

	if err = tx.Commit(); err != nil {
		return err
	}

	http.Redirect(w, r, "/dashboard/home", http.StatusFound)

	return nil
}
