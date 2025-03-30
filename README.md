# rfid-inventory-tracker

A system for automatically registering and tracking physical products using RFID tags

# Architecture

-   A central MQTT broker listens for requests from the reader/writer board
-   When registering new items, the operator will be given a sequential list of new items
    - New items are procedurally added via MQTT
    -   The reader/writer will iterate through a list of items, displaying the item name/ID on the LCD
    -   The operator will scan the corresponding RFID tag and confirm it via a button
    -   The reader/writer will send a request to the MQTT broker to update the item with the correct RFID tag
-   As new items travel through the entry gate, the reader/writer will:
    -   Read the corresponding RFID tag
    -   Send a request to the MQTT broker to update the inventory of the item
-   Operators can use the reader/writer board as a handheld device to:
    -   Scan any item's RFID tag
    -   Read information about the item
        -   Inventory
        -   Name
        -   ID
        -   Other metadata like total reads/writes, etc.

# Dashboard Requirements

-   May also be connected to SQL or use MQTT for item requests
-   Can view statistics about items, reads, writes, etc.
-   Telemetry
-   Can send new items to be associated with an RFID tag over MQTT to the writer
-   Can be deployed easily using IaC tooling

## Dashboard Stack
-   Golang
-   JWT
-   SQLC
-   PostgreSQL

# Broker Requirements

-   Connected to a SQL database
-   Serves an MQTT broker endpoint
-   Handles CRUD operations related to items
-   May also be used by an internal dashboard to view items

# Reader/Writer Requirements

-   One piece of hardware for both registering tracking, and monitoring products
    -   Can register new products by writing a key associated with an item to an RFID tag
    -   Can track products as a handheld device by reading RFID tags and pulling associated info
        -   Inventory of the item
        -   Item name
    -   Can monitor products by acting as an entry gate
        -   Reads new pallets or items as they come in
        -   Automatically updates the database with the correct inventory
    -   Must have a switch to change between modes
-   Supports both battery power & power via USB
-   Supports microSD for logging & configuration
-   An LCD screen to display info
-   Controlled by RPI Pico W

# Bill of Materials

| Part Number | Item Name | Purpose |
| ----------- | --------- | ------- |
|             |           |
|             |           |

# Database Structures

## Users Table

This table stores all information related to admins that can interface with the dashboard

| Column Name    | Type   | Description                                 | Flags                                            |
| -------------- | ------ | ------------------------------------------- | ------------------------------------------------ |
| Id             | Number | The ID of the user                          | Primary Key, Unique, Autoincrement               |
| Username       | String | The username of the user                    | Unique                                           |
| PasswordDigest | String | The hashed password                         |
| Permissions    | Number | The ID of the referenced permissions object | Foreign key, references permissions table row ID |

## Permissions Table

This table references the permissions a specific user has within the dashboard

| Column Name   | Type      | Description                                             | Flags                              |
| ------------- | --------- | ------------------------------------------------------- | ---------------------------------- |
| Id            | Number    | The ID of the permissions object                        | Primary Key, Unique, Autoincrement |
| ViewTelemetry | Bool      | If the user can view telemetry data                     |                                    |
| ViewItems     | Bool      | If the user can view item details                       |
| ManageItems   | Bool      | If the user can manage items (create, delete, etc.)     |                                    |
| PrepareTags   | Bool      | If the user can prepare RFID tag IDs for new items      |
| ManageUsers   | Bool      | If the user can manage other user permissions           |
| LastUpdated   | Timestamp | The timestamp the user permissions were last updated at |

## Items Table

This table contains the information for a specific item

| Column Name | Type   | Description          | Flags                              |
| ----------- | ------ | -------------------- | ---------------------------------- |
| Id          | Number | The ID of the item   | Primary Key, Unique, Autoincrement |
| Name        | String | The name of the item |
| Sku         | String | The item SKU         |
| Category    | String | The item category    |

## RFID Tags Table

This table has information about different RFID tags

-   Multiple RFID tags can be used for a single item
-   RFID tags have a specific quantity associated with them
    -   This allows for multiple items to have different quantities at the entry point
    -   e.g. Containers vs. pallets of the same item

| Column Name | Type   | Description                                              | Flags                              |
| ----------- | ------ | -------------------------------------------------------- | ---------------------------------- |
| Id          | Number | The ID of the item                                       | Primary Key, Unique, Autoincrement |
| RFIDId      | Number | The RFID Tag                                             | Foreign Key, references RFID ID    |
| ItemId      | Number | The corresponding item ID                                | Foreign Key, references item ID    |
| Quantity    | Number | The number of items corresponding with this RFID tag     |
| TagType     | String | The tag type (pallet, container-full) used for telemetry |

## Transaction Table

This table tracks all transactions that have to do with a specific item

-   References both an item and RFID tag
-   Tracks both the new quantity and the changed amount of the item stock
-   Knowledge of the direction the transaction ocurred in (IN/OUT of the warehouse)
-   Knowledge of where the transaction ocurred at (e.g. mutltiple entry gates/stations)
-   Sorted by latest transaction
-   Must be easily modifiable by the dashboard to ensure that mistakes can be corrected quickly

| Column Name          | Type      | Description                                           | Flags                              |
| -------------------- | --------- | ----------------------------------------------------- | ---------------------------------- |
| Id                   | Number    | The ID of the item entry                              | Primary Key, Unique, Autoincrement |
| RFIDId               | Number    | The RFID Tag                                          | Foreign Key, references RFID ID    |
| ItemId               | Number    | The corresponding item ID                             | Foreign Key, references item ID    |
| QuantityChanged      | Number    | The signed quantity that changed                      |
| NewQuantity          | Number    | The new quantity of the stored item                   |
| TransactionDirection | String    | The direction the transaction was headed in (IN, OUT) |
| Source               | String    | The place the transaction ocurred at                  |
| Timestamp            | Timestamp | The timestamp at which the transaction ocurred        |
