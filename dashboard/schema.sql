CREATE TABLE goose_db_version (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		version_id INTEGER NOT NULL,
		is_applied INTEGER NOT NULL,
		tstamp TIMESTAMP DEFAULT (datetime('now'))
	);
CREATE TABLE sqlite_sequence(name,seq);
CREATE TABLE users (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Username text UNIQUE,
    PasswordDigest text,
    Permissions INTEGER,
    FOREIGN KEY(Permissions) REFERENCES permissions(Id)
);
CREATE TABLE permissions (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    ViewTelemetry BOOLEAN,
    ViewItems BOOLEAN,
    ManageItems BOOLEAN,
    PrepareTags BOOLEAN,
    ManageUsers BOOLEAN,
    LastUpdated DATETIME DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE items (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Name text,
    Sku text,
    Category text
);
CREATE TABLE tags (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Uid text,
    Item INTEGER,
    Quantity INTEGER,
    TagType text,
    FOREIGN KEY(Item) REFERENCES items(Id)
);
CREATE TABLE transactions (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    Tag INTEGER,
    Item INTEGER,
    QuantityChanged INTEGER,
    NewQuantity INTEGER,
    TransactionDirection text,
    Source text,
    "Timestamp" DATETIME DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY(Tag) REFERENCES tags(Id),
    FOREIGN KEY(Item) REFERENCES items(Id)
);
