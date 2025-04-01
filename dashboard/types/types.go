package types

import (
	"context"

	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/jmoiron/sqlx"
	"github.com/matthewhartstonge/argon2"
)

type Config struct {
	Port             int    `env:"PORT"`
	DatabaseUrl      string `env:"DATABASE_URL"`
	CookieDomain     string `env:"COOKIE_DOMAIN"`
	JwtSigningSecret string `env:"JWT_SIGNING_SECRET"`
}

type RequestContext struct {
	DB           *sqlx.DB
	Repositories *repositories.Queries
	Ctx          context.Context
	Config       *Config
	Argon2       *argon2.Config
}

type LoginRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

type SignupRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
}
