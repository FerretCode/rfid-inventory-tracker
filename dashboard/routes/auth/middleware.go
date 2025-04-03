package auth

import (
	"context"
	"errors"
	"fmt"
	"log/slog"
	"net/http"
	"time"

	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/golang-jwt/jwt/v5"
)

func verifyJWT(tokenStr string, config *types.Config) (jwt.MapClaims, error) {
	token, err := jwt.Parse(tokenStr, func(token *jwt.Token) (interface{}, error) {
		if token.Method != jwt.SigningMethodHS256 {
			return nil, fmt.Errorf("unexpected signing method: %v", token.Header["alg"])
		}
		return []byte(config.JwtSigningSecret), nil
	})
	if err != nil {
		return nil, err
	}

	claims := token.Claims.(jwt.MapClaims)

	if token.Valid {
		if expiration, ok := claims["exp"].(float64); ok {
			expirationTime := time.Unix(int64(expiration), 0)

			if time.Now().After(expirationTime) {
				return nil, errors.New("token has expired")
			}
		} else {
			return nil, errors.New("could not parse expiration time")
		}

		return claims, nil
	}

	return nil, errors.New("invalid token claims")
}

func CheckAuth(config *types.Config, repositories *repositories.Queries, logger *slog.Logger) func(next http.Handler) http.Handler {
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

			if userId, ok := claims["user_id"].(float64); ok {
				user, err := repositories.GetUser(r.Context(), int64(userId))
				if err != nil {
					http.Error(w, "error getting user", http.StatusInternalServerError)
					return
				}

				permissions, err := repositories.GetPermission(r.Context(), user.Permissions.Int64)
				if err != nil {
					http.Error(w, "error getting user permissions", http.StatusInternalServerError)
					return
				}

				ctx := context.WithValue(r.Context(), "user", user)
				ctx = context.WithValue(ctx, "permission", permissions)

				next.ServeHTTP(w, r.WithContext(ctx))

				return
			}

			http.Error(w, "there was an error authenticating you", http.StatusInternalServerError)
		})
	}
}
