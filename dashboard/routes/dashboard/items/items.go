package items

import (
	"database/sql"
	"encoding/json"
	"html/template"
	"net/http"
	"strconv"

	"github.com/ferretcode/rfid-inventory-tracker/repositories"
	"github.com/ferretcode/rfid-inventory-tracker/types"
	"github.com/go-chi/chi/v5"
)

type webItem struct {
	ID       int64
	Category string
	Name     string
	Sku      string
}

type itemsData struct {
	User       repositories.User
	Permission repositories.Permission
	Items      []webItem
}

func Items(w http.ResponseWriter, r *http.Request, templates *template.Template, queries *repositories.Queries) error {
	items, err := queries.ListItems(r.Context())
	if err != nil {
		return err
	}

	var webItems []webItem

	for _, item := range items {
		webItems = append(webItems, webItem{
			ID:       item.ID,
			Name:     item.Name.String,
			Sku:      item.Sku.String,
			Category: item.Category.String,
		})
	}

	err = templates.ExecuteTemplate(w, "items.html", itemsData{
		User:       r.Context().Value("user").(repositories.User),
		Permission: r.Context().Value("permission").(repositories.Permission),
		Items:      webItems,
	})
	if err != nil {
		return err
	}

	return nil
}

func CreateItem(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
	var item repositories.CreateItemParams

	err := r.ParseForm()
	if err != nil {
		return err
	}

	item.Name = sql.NullString{r.PostFormValue("name"), true}
	item.Category = sql.NullString{r.PostFormValue("category"), true}
	item.Sku = sql.NullString{r.PostFormValue("sku"), true}

	tx, err := ctx.DB.Begin()
	if err != nil {
		return err
	}
	defer tx.Rollback()

	qtx := ctx.Repositories.WithTx(tx)

	newItem, err := qtx.CreateItem(ctx.Ctx, item)
	if err != nil {
		return err
	}

	if err = tx.Commit(); err != nil {
		return err
	}

	messagePayload := types.ItemRegistrationPaylod{
		ItemId:      newItem.ID,
		ItemName:    newItem.Name.String,
		TagQuantity: 100, // TODO: make this variable
	}

	stringified, err := json.Marshal(messagePayload)
	if err != nil {
		return err
	}

	token := ctx.MQTTConn.Publish("item_registration", 1, false, stringified)
	if token.Wait() && token.Error() != nil {
		return err
	}

	http.Redirect(w, r, "/dashboard/items", http.StatusFound)

	return nil
}

// TODO: implement
func DeleteItem(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
	return nil
}

// TODO: implement
func UpdateItem(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
	return nil
}

func GetItem(w http.ResponseWriter, r *http.Request, ctx types.RequestContext) error {
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
