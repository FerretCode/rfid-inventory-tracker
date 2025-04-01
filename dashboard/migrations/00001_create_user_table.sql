-- +goose Up
-- +goose StatementBegin
CREATE TABLE users (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Username text UNIQUE,
    PasswordDigest text,
    Permissions INTEGER,
    FOREIGN KEY(Permissions) REFERENCES permissions(Id)
);
-- +goose StatementEnd

-- +goose Down
-- +goose StatementBegin
DROP TABLE users
-- +goose StatementEnd
