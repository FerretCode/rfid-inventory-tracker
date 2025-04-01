package auth

import (
	"context"
	"errors"
	"fmt"
	"net/http"
	"time"

	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/golang-jwt/jwt/v5"
)

func verifyJWT(tokenStr string, config *types.Config) (jwt.MapClaims, error) {
	token, err := jwt.Parse(tokenStr, func(token *jwt.Token) (interface{}, error) {
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, fmt.Errorf("unexpected signing method: %v", token.Header["alg"])
		}
		return []byte(config.JwtSigningSecret), nil
	})
	if err != nil {
		return nil, err
	}

	if claims, ok := token.Claims.(jwt.MapClaims); ok && token.Valid {
		if expiration, ok := claims["exp"].(int64); ok {
			expirationTime := time.Unix(expiration, 0)

			if time.Now().After(expirationTime) {
				return nil, errors.New("token has expired")
			}
		}

		return claims, nil
	}

	return nil, errors.New("invalid token claims")
}

func CheckAuth(config *types.Config, repositories *repositories.Queries) func(next http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {

		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			cookie, err := r.Cookie("dashboard")
			if err != nil {
				http.Error(w, "unauthorized", http.StatusUnauthorized)
				return
			}

			claims, err := verifyJWT(cookie.Value, config)
			if err != nil {
				http.Error(w, "invalid token", http.StatusUnauthorized)
				return
			}

			user, err := repositories.GetUser(r.Context(), int64(claims["user_id"].(float64)))
			if err != nil {
				http.Error(w, "error getting user", http.StatusInternalServerError)
				return
			}

			fmt.Println(user)

			ctx := context.WithValue(r.Context(), "user", user)

			next.ServeHTTP(w, r.WithContext(ctx))
		})
	}
}
