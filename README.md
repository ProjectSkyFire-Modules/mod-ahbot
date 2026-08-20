## Auction Bot (SkyFire port)

This is a SkyFire 5.4.8 port of [NathanHandley/mod-ah-bot-plus](https://github.com/NathanHandley/mod-ah-bot-plus), a config-driven auction house seller/buyer bot. All tuning is done in `mod_ahbot.conf`; no SQL setup is required.

Feature highlights:
- Listing control (include/exclude rules, category proportions, stack sizes)
- Customizable pricing by category, quality, item level, and optional advanced formulas
- Optional buyer bot
- Multiple bot characters so listings show different owner names
- GM commands to reload config, force an update, or empty bot auctions

## Installation

The module lives at `modules/mod-ahbot` and is compiled automatically with the SkyFire module system. Re-run CMake and rebuild worldserver.

Copy `mod_ahbot.conf.dist` next to `worldserver.conf` as `mod_ahbot.conf` (the build already stages the `.dist` file beside the worldserver binary).

## Usage

**Create at least one unused character** whose name will appear on listings, then set `AuctionHouseBot.GUIDs` to that character's GUID from the `characters` table. Then set `AuctionHouseBot.EnableSeller = true`. Enable the buyer with `AuctionHouseBot.Buyer.Enabled`.

These characters are dummy listing owners only. They are not playerbots and should never log in or walk to an auctioneer. Playerbots / AI characters are not meant to use the auction house; this module is the only thing that posts and buys listings to fill the market.

Notes:
- Do not log in on these characters, and do not reuse playerbot character GUIDs.
- Auction mail to the dummy characters is deleted by the module (expired items can optionally be kept with `AuctionHouseBot.ReturnExpiredAuctionItemsToBot`).
- It takes several hours to fully populate the auction house at the default 75–150 items per cycle.
- Bot buyouts are capped at 100k gold unless you change `AuctionHouseBot.MaxBuyoutPriceInCopper`.
- SkyFire stores auctions against a real auctioneer NPC spawn. Leave `AuctionHouseBot.*.AuctioneerGuid` at `0` to auto-pick Alliance/Horde/Neutral auctioneers, or set spawn GUIDs explicitly.

### In-Game Commands

| Command | Description |
|----------|-------------|
| `.ahbot reload` | Reloads `mod_ahbot.conf` and rebuilds item candidate lists |
| `.ahbot empty` | Removes all bot auctions. Player auctions are untouched; bids are refunded |
| `.ahbot update` | Runs one buy/sell cycle immediately |

## Credits

- NathanHandley: rewrite of the AzerothCore auction house bot
- Zeb139: pricing and performance work
- Ayase: original AzerothCore port
- SkyFire port: adapted to 5.4.8 auction, loot, config, and command APIs
