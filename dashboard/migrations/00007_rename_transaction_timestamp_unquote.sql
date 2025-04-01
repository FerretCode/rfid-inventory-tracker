-- +goose Up
-- +goose StatementBegin
ALTER TABLE transactions
RENAME COLUMN "Timestamp" TO Timestamp
-- +goose StatementEnd

-- +goose Down
-- +goose StatementBegin
ALTER TABLE transactions
RENAME COLUMN Timestamp TO "Timestamp"
-- +goose StatementEnd
