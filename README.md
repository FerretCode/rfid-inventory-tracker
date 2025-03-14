# rfid-inventory-tracker

A system for automatically registering and tracking physical products using RFID tags

# Architecture

-   A central MQTT broker listens for requests from the reader/writer board
-   When registering new items, the operator will be given a sequential list of new items
    -   New items are supplied via an SD card
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
-   Can also generate item lists for writing to write to the microSD card on the writer

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
