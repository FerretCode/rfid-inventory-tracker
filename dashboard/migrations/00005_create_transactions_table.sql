-- +goose Up
-- +goose StatementBegin
CREATE TABLE transactions (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Tag INTEGER,
    Item INTEGER,
    QuantityChanged INTEGER,
    NewQuantity INTEGER,
    TransactionDirection text,
    Source text,
    LastUpdated DATETIME DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY(Tag) REFERENCES tags(Id),
    FOREIGN KEY(Item) REFERENCES items(Id)
);
-- +goose StatementEnd

-- +goose Down
-- +goose StatementBegin
SELECT 'down SQL query';
-- +goose StatementEnd
