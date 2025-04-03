package main

import (
	"context"
	"fmt"
	"html/template"
	"log"
	"log/slog"
	"net/http"
	"os"
	"time"

	"github.com/caarlos0/env/v11"
	"github.com/eclipse/paho.mqtt.golang"
	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/ferretcode/rfid-inventory-tracker/routes/auth"
	"github.com/ferretcode/rfid-inventory-tracker/routes/dashboard"
	"github.com/ferretcode/rfid-inventory-tracker/routes/dashboard/items"
	"github.com/ferretcode/rfid-inventory-tracker/routes/dashboard/tags"
	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
	"github.com/jmoiron/sqlx"
	"github.com/joho/godotenv"
	"github.com/matthewhartstonge/argon2"
	_ "github.com/mattn/go-sqlite3"
)

var config types.Config
var logger *slog.Logger
var templates *template.Template

func parseTemplates() error {
	var err error

	files := []string{
		"./views/fragments/navbar.html",
		"./views/error.html",
		"./views/dashboard/home.html",
		"./views/dashboard/items.html",
		"./views/dashboard/tags.html",
		"./views/auth/signup.html",
		"./views/auth/login.html",
	}

	templates, err = template.ParseFiles(files...)
	if err != nil {
		return err
	}

	return nil
}

func main() {
	logger = slog.New(slog.NewJSONHandler(os.Stdout, nil))
	slog.SetDefault(logger)

	if _, err := os.Stat(".env"); err == nil {
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

	mqtt.DEBUG = log.New(os.Stdout, "MQTTDebug: ", 0)
	mqtt.ERROR = log.New(os.Stderr, "MQTTError: ", 0)

	opts := mqtt.
		NewClientOptions().
		AddBroker(fmt.Sprintf("tcp://%s:%d", config.MQTTBrokerHost, config.MQTTBrokerPort)).
		SetClientID("dashboard").
		SetKeepAlive(60 * time.Second).
		SetPingTimeout(1 * time.Second)

	c := mqtt.NewClient(opts)
	if token := c.Connect(); token.Wait() && token.Error() != nil {
		slog.Error("error connecting to mqtt broker", "err", token.Error())
		return
	}

	err = parseTemplates()
	if err != nil {
		slog.Error("error parsing html templates", "err", err)
		return
	}

	repositories := repositories.New(conn)
	argon2 := argon2.DefaultConfig()

	mqttHandler := tags.NewMQTTHandler(repositories, conn, &ctx, logger)

	c.Subscribe("item_registration_completed", 1, mqttHandler.CreateTag)
	requestContext := types.RequestContext{
		DB:           conn,
		Repositories: repositories,
		Ctx:          ctx,
		Config:       &config,
		Argon2:       &argon2,
		MQTTConn:     c,
	}

	r := chi.NewRouter()

	r.Use(middleware.Logger)
	r.Use(middleware.RealIP)
	r.Use(middleware.Recoverer)

	r.Route("/auth", func(r chi.Router) {
		r.Get("/signup", func(w http.ResponseWriter, r *http.Request) {
			handleError(auth.RenderSignupTemplate(w, r, templates), w, "signup/render")
		})

		r.Get("/login", func(w http.ResponseWriter, r *http.Request) {
			handleError(auth.RenderLoginTemplate(w, r, templates), w, "login/render")
		})

		r.Post("/signup", func(w http.ResponseWriter, r *http.Request) {
			handleError(auth.Signup(w, r, requestContext), w, "signup/post")
		})

		r.Post("/login", func(w http.ResponseWriter, r *http.Request) {
			handleError(auth.Login(w, r, requestContext), w, "login/post")
		})
	})

	r.Route("/dashboard", func(r chi.Router) {
		r.Use(auth.CheckAuth(&config, repositories, logger))
		r.Get("/", func(w http.ResponseWriter, r *http.Request) {
			http.Redirect(w, r, "/dashboard/home", http.StatusFound)
		})

		r.Get("/home", func(w http.ResponseWriter, r *http.Request) {
			handleError(dashboard.Home(w, r, templates), w, "dashboard/home")
		})

		r.Route("/items", func(r chi.Router) {
			r.Get("/", func(w http.ResponseWriter, r *http.Request) {
				handleError(items.Items(w, r, templates, repositories), w, "items/render")
			})

			r.Post("/create", func(w http.ResponseWriter, r *http.Request) {
				handleError(items.CreateItem(w, r, requestContext), w, "items/create")
			})

			r.Get("/get/{item_id}", func(w http.ResponseWriter, r *http.Request) {
				handleError(items.GetItem(w, r, requestContext), w, "items/get")
			})

			r.Post("/update", func(w http.ResponseWriter, r *http.Request) {})
			r.Post("/delete", func(w http.ResponseWriter, r *http.Request) {})
		})

		r.Route("/tags", func(r chi.Router) {
			r.Get("/", func(w http.ResponseWriter, r *http.Request) {
				handleError(tags.Tags(w, r, templates, repositories), w, "tags/render")
			})

			r.Get("/get/{tag_id}", func(w http.ResponseWriter, r *http.Request) {})
			r.Post("/update", func(w http.ResponseWriter, r *http.Request) {})
			r.Post("/delete", func(w http.ResponseWriter, r *http.Request) {})
		})
	})

	http.ListenAndServe(fmt.Sprintf(":%d", config.Port), r)
}

func handleError(err error, w http.ResponseWriter, svc string) {
	if err != nil {
		http.Error(w, "there was an error processing your request", http.StatusInternalServerError)
		logger.Error("error processing request", "svc", svc, "err", err)
	}
}
