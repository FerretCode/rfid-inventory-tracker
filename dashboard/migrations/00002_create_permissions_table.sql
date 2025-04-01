-- +goose Up
-- +goose StatementBegin
CREATE TABLE permissions (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    ViewTelemetry BOOLEAN,
    ViewItems BOOLEAN,
    ManageItems BOOLEAN,
    PrepareTags BOOLEAN,
    ManageUsers BOOLEAN,
    LastUpdated DATETIME DEFAULT CURRENT_TIMESTAMP
);
-- +goose StatementEnd

-- +goose Down
-- +goose StatementBegin
DROP TABLE permissions
-- +goose StatementEnd
