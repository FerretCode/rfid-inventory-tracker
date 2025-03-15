package main

import (
	"log/slog"
	"os"

	"github.com/caarlos0/env/v11"
	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
	"github.com/joho/godotenv"
)

var config types.Config

func main() {
	logger := slog.New(slog.NewJSONHandler(os.Stdout, nil))
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

	r := chi.NewRouter()

	r.Use(middleware.Logger)
	r.Use(middleware.RealIP)
	r.Use(middleware.Recoverer)
}
