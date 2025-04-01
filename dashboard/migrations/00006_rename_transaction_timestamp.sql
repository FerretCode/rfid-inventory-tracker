-- +goose Up
-- +goose StatementBegin
ALTER TABLE transactions
RENAME COLUMN LastUpdated TO "Timestamp"
-- +goose StatementEnd

-- +goose Down
-- +goose StatementBegin
ALTER TABLE transactions
RENAME COLUMN "Timestamp" TO LastUpdated
-- +goose StatementEnd
