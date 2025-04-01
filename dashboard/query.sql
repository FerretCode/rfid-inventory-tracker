-- name: GetUser :one
SELECT * FROM users
WHERE Id = ? LIMIT 1;

-- name: GetUserByUsername :one
SELECT * FROM users
WHERE Username = ? LIMIT 1;

-- name: ListUsers :many
SELECT * FROM users
ORDER BY Id;

-- name: CreateUser :one
INSERT INTO users (
    Id,
    Username,
    PasswordDigest,
    Permissions
) VALUES (
    ?, ?, ?, ?
) RETURNING *;

-- name: UpdateUser :exec
UPDATE users
set Username = ?,
    PasswordDigest = ?,
    Permissions = ?
WHERE Id = ?;

-- name: DeleteUser :exec
DELETE FROM users
WHERE Id = ?;

-- name: GetPermission :one
SELECT * FROM permissions
WHERE Id = ? LIMIT 1;

-- name: GetPermissions :many
SELECT * FROM permissions
ORDER BY Id;

-- name: CreatePermission :one
INSERT INTO permissions (
    Id,
    ViewTelemetry,
    ViewItems,
    ManageItems,
    PrepareTags,
    ManageUsers
) VALUES (
    ?, ?, ?, ?, ?, ?
) RETURNING *;

-- name: UpdatePermission :exec
UPDATE permissions
set ViewTelemetry = ?,
    ViewItems = ?,
    ManageItems = ?,
    PrepareTags = ?,
    ManageUsers = ?,
    LastUpdated = ?
WHERE Id = ?;

-- name: DeletePermission :exec
DELETE FROM permissions
WHERE Id = ?;

-- name: GetItem :one
SELECT * FROM items
WHERE Id = ? LIMIT 1;

-- name: ListItems :many
SELECT * FROM items
ORDER BY Id;

-- name: CreateItem :one
INSERT INTO items (
    Id,
    Name,
    Sku,
    Category
) VALUES (
    ?, ?, ?, ?
) RETURNING *;

-- name: UpdateItem :exec
UPDATE items
set Name = ?,
    Sku = ?,
    Category = ?
WHERE Id = ?;

-- name: DeleteItem :exec
DELETE FROM items
WHERE Id = ?;

-- name: GetTag :one
SELECT * FROM tags
WHERE Id = ? LIMIT 1;

-- name: ListTags :many
SELECT * FROM tags
ORDER BY Id;

-- name: CreateTag :one
INSERT INTO tags (
    Id,
    Uid,
    Item,
    Quantity,
    TagType
) VALUES (
    ?, ?, ?, ?, ?
) RETURNING *;

-- name: UpdateTag :exec
UPDATE tags
set Uid = ?,
    Item = ?,
    Quantity = ?,
    TagType = ?
WHERE Id = ?;

-- name: DeleteTag :exec
DELETE FROM tags
WHERE Id = ?;

-- name: GetTransaction :one
SELECT * FROM transactions
WHERE Id = ? LIMIT 1;

-- name: ListTransactions :many
SELECT * FROM transactions
ORDER BY Id;

-- name: CreateTransaction :one
INSERT INTO transactions (
    Id,
    Tag,
    Item,
    QuantityChanged,
    NewQuantity,
    TransactionDirection,
    Source,
    "Timestamp"
) VALUES (
    ?, ?, ?, ?, ?, ?, ?, ?
) RETURNING *;

-- name: UpdateTransaction :exec
UPDATE transactions
set Tag = ?,
    Item = ?,
    QuantityChanged = ?,
    NewQuantity = ?,
    TransactionDirection = ?,
    Source = ?,
    "Timestamp" = ?
WHERE Id = ?;

-- name: DeleteTransaction :exec
DELETE FROM transactions
WHERE Id = ?;

