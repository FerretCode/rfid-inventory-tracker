package dashboard

import (
	"html/template"
	"net/http"

	"github.com/ferretcode/rfid-inventory-tracker/repositories"
)

type homeData struct {
	User       repositories.User
	Permission repositories.Permission
}

func Home(w http.ResponseWriter, r *http.Request, templates *template.Template) error {
	err := templates.ExecuteTemplate(w, "home.html", homeData{
		User:       r.Context().Value("user").(repositories.User),
		Permission: r.Context().Value("permission").(repositories.Permission),
	})
	if err != nil {
		return err
	}

	return nil
}
