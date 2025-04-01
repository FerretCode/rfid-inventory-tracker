-- +goose Up
CREATE TABLE items (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Name text,
    Sku text,
    Category text
);
-- +goose StatementBegin

-- +goose StatementEnd

-- +goose Down
-- +goose StatementBegin
DROP TABLE items;
-- +goose StatementEnd
