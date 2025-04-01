package main

import (
	"context"
	"log/slog"
	"net/http"
	"os"

	"github.com/caarlos0/env/v11"
	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/ferretcode/rfid-inventory-tracker/routes/auth"
	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
	"github.com/jmoiron/sqlx"
	"github.com/joho/godotenv"
	_ "github.com/mattn/go-sqlite3"
)

var config types.Config
var logger *slog.Logger

func main() {
	logger = slog.New(slog.NewJSONHandler(os.Stdout, nil))
	slog.SetDefault(logger)

	if _, err := os.Stat(".env"); err != nil {
		if err := godotenv.Load(".env"); err != nil {
			slog.Error("error loading .env", "err", err)
			return
		}
	}

	if err := env.Parse(&config); err != nil {
		slog.Error("error parsing .env", "err", err)
		return
	}

	ctx := context.Background()

	conn, err := sqlx.Open("sqlite3", config.DatabaseUrl)
	if err != nil {
		slog.Error("error opening database connection", "err", err)
		return
	}
	defer conn.Close()

	repositories := repositories.New(conn)

	requestContext := types.RequestContext{
		DB:           conn,
		Repositories: repositories,
		Ctx:          ctx,
		Config:       &config,
	}

	r := chi.NewRouter()

	r.Use(middleware.Logger)
	r.Use(middleware.RealIP)
	r.Use(middleware.Recoverer)

	r.Route("/", func(r chi.Router) {
		r.Route("/auth", func(r chi.Router) {
			r.Post("/signup", func(w http.ResponseWriter, r *http.Request) {
				handleError(auth.Signup(w, r, requestContext), w, "signup")
			})

			r.Post("/login", func(w http.ResponseWriter, r *http.Request) {

			})
		})
	})
}

func handleError(err error, w http.ResponseWriter, svc string) {
	if err != nil {
		http.Error(w, "there was an error processing your request", http.StatusInternalServerError)
		logger.Error("error processing request", "svc", svc, "err", err)
	}
}
