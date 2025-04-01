-- +goose Up
-- +goose StatementBegin
CREATE TABLE tags (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Uid text,
    Item INTEGER,
    Quantity INTEGER,
    TagType text,
    FOREIGN KEY(Item) REFERENCES items(Id)
);
-- +goose StatementEnd

-- +goose Down
-- +goose StatementBegin
DROP TABLE tags
-- +goose StatementEnd
