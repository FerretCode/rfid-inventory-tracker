package auth

import (
	"database/sql"
	"encoding/json"
	"io"
	"net/http"
	"time"

	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/golang-jwt/jwt/v5"
	"github.com/matthewhartstonge/argon2"
)

func Signup(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
	signupRequest := types.SignupRequest{}

	body, err := io.ReadAll(r.Body)
	if err != nil {
		return err
	}

	if err = json.Unmarshal(body, &signupRequest); err != nil {
		return err
	}

	argon := argon2.DefaultConfig()

	digest, err := argon.HashEncoded([]byte(signupRequest.Password))
	if err != nil {
		return err
	}

	tx, err := ctx.DB.Begin()
	if err != nil {
		return err
	}
	defer tx.Rollback()

	qtx := ctx.Repositories.WithTx(tx)

	existingUsers, err := qtx.ListUsers(ctx.Ctx)
	if err != nil {
		return err
	}

	var permissionParams repositories.CreatePermissionParams

	// create admin account
	if len(existingUsers) == 0 {
		permissionParams = repositories.CreatePermissionParams{
			Viewtelemetry: sql.NullBool{true, true},
			Viewitems:     sql.NullBool{true, true},
			Manageitems:   sql.NullBool{true, true},
			Preparetags:   sql.NullBool{true, true},
			Manageusers:   sql.NullBool{true, true},
		}
	} else {
		permissionParams = repositories.CreatePermissionParams{
			Viewtelemetry: sql.NullBool{false, true},
			Viewitems:     sql.NullBool{false, true},
			Manageitems:   sql.NullBool{false, true},
			Preparetags:   sql.NullBool{false, true},
			Manageusers:   sql.NullBool{false, true},
		}
	}

	permissions, err := qtx.CreatePermission(ctx.Ctx, permissionParams)
	if err != nil {
		return err
	}

	_, err = qtx.CreateUser(ctx.Ctx, repositories.CreateUserParams{
		Username:       sql.NullString{signupRequest.Username, true},
		Passworddigest: sql.NullString{string(digest), true},
		Permissions:    sql.NullInt64{permissions.ID, true},
	})
	if err != nil {
		return err
	}

	claims := jwt.MapClaims{
		"username": signupRequest.Username,
		"exp":      time.Now().Add(24 * time.Hour).Unix(),
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)

	signedToken, err := token.SignedString(ctx.Config.JwtSigningSecret)
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

	w.WriteHeader(200)
	http.Redirect(w, r, "/dashboard", http.StatusFound)

	return nil
}
