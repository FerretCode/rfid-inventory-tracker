package tags

import (
	"context"
	"database/sql"
	"encoding/json"
	"html/template"
	"log/slog"
	"net/http"
	"strconv"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/go-chi/chi/v5"
	"github.com/jmoiron/sqlx"
)

type MQTTHandler struct {
	Repositories repositories.Queries
	DB           *sqlx.DB
	Ctx          *context.Context
	Logger       *slog.Logger
}

type webTag struct {
	ID       int64
	Uid      string
	Item     int64
	Quantity int64
	TagType  string
}

type tagsData struct {
	User       repositories.User
	Permission repositories.Permission
	Tags       []webTag
}

func NewMQTTHandler(repositories *repositories.Queries, db *sqlx.DB, ctx *context.Context, logger *slog.Logger) MQTTHandler {
	return MQTTHandler{
		Repositories: *repositories,
		DB:           db,
		Ctx:          ctx,
		Logger:       logger,
	}
}

func Tags(w http.ResponseWriter, r *http.Request, templates *template.Template, queries *repositories.Queries) error {
	tags, err := queries.ListTags(r.Context())
	if err != nil {
		return err
	}

	var webTags []webTag

	for _, tag := range tags {
		webTags = append(webTags, webTag{
			ID:       tag.ID,
			Uid:      tag.Uid.String,
			Item:     tag.Item.Int64,
			Quantity: tag.Quantity.Int64,
			TagType:  tag.Tagtype.String,
		})
	}

	err = templates.ExecuteTemplate(w, "tags.html", tagsData{
		User:       r.Context().Value("user").(repositories.User),
		Permission: r.Context().Value("permission").(repositories.Permission),
		Tags:       webTags,
	})
	if err != nil {
		return err
	}

	return nil
}

func (h *MQTTHandler) CreateTag(client mqtt.Client, msg mqtt.Message) {
	createTagRequest := types.CreateTagRequest{}

	if err := json.Unmarshal(msg.Payload(), &createTagRequest); err != nil {
		h.Logger.Error("error decoding payload", "err", err)
		return
	}

	h.Logger.Info("new item finished registration", "request", createTagRequest)

	tx, err := h.DB.Begin()
	if err != nil {
		h.Logger.Error("error opening database transaction", "err", err)
		return
	}
	defer tx.Rollback()

	qtx := h.Repositories.WithTx(tx)

	_, err = qtx.CreateTag(*h.Ctx, repositories.CreateTagParams{
		Uid:      sql.NullString{createTagRequest.Uid, true},
		Item:     sql.NullInt64{createTagRequest.ItemId, true},
		Quantity: sql.NullInt64{createTagRequest.Quantity, true},
	})
	if err != nil {
		h.Logger.Error("error creating tag", "err", err)
		return
	}

	if err = tx.Commit(); err != nil {
		h.Logger.Error("error committing database transaction", "err", err)
		return
	}
}

// TODO: implement
func DeleteItem(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
	return nil
}

// TODO: implement
func UpdateItem(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
	return nil
}

func GetTag(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
	itemId := chi.URLParam(r, "item_id")

	id, err := strconv.Atoi(itemId)
	if err != nil {
		return err
	}

	item, err := ctx.Repositories.GetItem(ctx.Ctx, int64(id))
	if err != nil {
		return err
	}

	stringified, err := json.Marshal(item)
	if err != nil {
		return err
	}

	w.WriteHeader(200)
	w.Write(stringified)

	return nil
}
