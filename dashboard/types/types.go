package types

import (
	"context"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/jmoiron/sqlx"
	"github.com/matthewhartstonge/argon2"
)

type Config struct {
	Port             int    `env:"PORT"`
	DatabaseUrl      string `env:"DATABASE_URL"`
	CookieDomain     string `env:"COOKIE_DOMAIN"`
	JwtSigningSecret string `env:"JWT_SIGNING_SECRET"`
	MQTTBrokerHost   string `env:"MQTT_BROKER_HOST"`
	MQTTBrokerPort   int    `env:"MQTT_BROKER_PORT"`
}

type RequestContext struct {
	DB           *sqlx.DB
	Repositories *repositories.Queries
	Ctx          context.Context
	Config       *Config
	Argon2       *argon2.Config
	MQTTConn     mqtt.Client
}

type SignupRequest struct {
	Username string
	Password string
}

type LoginRequest struct {
	Username string
	Password string
}

type CreateTagRequest struct {
	Uid      []byte `json:"tag"`
	Quantity int64  `json:"tag_quantity"`
	ItemId   int64  `json:"item_id"`
}

type ItemRegistrationPaylod struct {
	ItemId      int64  `json:"item_id"`
	ItemName    string `json:"item_name"`
	TagQuantity int64  `json:"tag_quantity"`
}
