/*
 *  SeedCatalog.c
 *  Brogue
 *
 *  Copyright 2012. All rights reserved.
 *
 *  This file is part of Brogue.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Rogue.h"
#include "IncludeGlobals.h"

void printSeedCatalogCsvLine(unsigned long seed, short depth, short quantity, char categoryName[50], char kindName[50],
                             char enchantment[50], char runicName[50], char vaultNumber[10], char opensVaultNumber[10],
                             char carriedByMonsterName[50], char monsterDropNumber[10]){

    printf("%s,%lu,%i,%i,%s,%s,%s,%s,%s,%s,%s,%s\n", BROGUE_DUNGEON_VERSION_STRING, seed, depth, quantity, categoryName,
           kindName, enchantment, runicName, vaultNumber, opensVaultNumber, carriedByMonsterName, monsterDropNumber);

}

void printSeedCatalogFloorGold(int gold, short piles, boolean csvOutput) {
    char kindName[50] = "";
    
    if (csvOutput) {
        if (piles == 1) {
            strcpy(kindName, "gold pieces");
        } else if (piles > 1) {
            sprintf(kindName, "gold pieces (%i piles)", piles);
        }
        printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, gold, "gold", kindName, "","","","","","");
    } else {
        if (piles == 1) {
            printf("        %i gold pieces\n", gold);
        } else if (piles > 1) {
            printf("        %i gold pieces (%i piles)\n", gold, piles);
        }
    }
}

void printSeedCatalogItem(item *theItem, creature *theMonster, boolean csvOutput, short dropNumber) {
    char inGameItemName[128] = "", carriedByMonsterName[50] = "", vaultNumber[32] = "", opensVaultNumber[32] = "";
    char categoryName[20] = "", kindName[50] = "", enchantment[5] = "", runicName[30] = "", monsterDropNumber[8] = ""; 
    short depth = 0;

    if (csvOutput) {     //for csvOutput we need the item name components: category, kind, enchantment, & runic
        strcpy(categoryName, itemCategoryNames[theItem->kind]);
        itemKindName(theItem, kindName);
        itemRunicName(theItem, runicName);
        if (theItem->category & (ARMOR | CHARM | RING | STAFF | WAND | WEAPON)) {   //enchantable items
            if (theItem->category == WAND) {
                sprintf(enchantment, "%i", theItem->charges);
            } else {
                sprintf(enchantment, "%i", theItem->enchant1);
            }
        }
    } else {
        itemName(theItem, inGameItemName, true, true, NULL);   //for non-csv output, use the in-game item name as base
    }

    if (theMonster != NULL) {   //carried by monster
        sprintf(carriedByMonsterName, csvOutput ? "%s" : " (%s)", theMonster->info.monsterName);
    }

    if (pmap[theItem->xLoc][theItem->yLoc].machineNumber > 0) {     //item is in a vault
        //not all machines are "vaults" so we need to exclude some.
        if (pmap[theItem->xLoc][theItem->yLoc].layers[0] != ALTAR_SWITCH
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != ALTAR_SWITCH_RETRACTING
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != ALTAR_CAGE_RETRACTABLE
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != ALTAR_INERT
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != AMULET_SWITCH
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != FLOOR) {

            sprintf(vaultNumber, csvOutput ? "%i" : " (vault %i)", pmap[theItem->xLoc][theItem->yLoc].machineNumber);
        }
    }

    if (theItem->category == KEY && theItem->kind == KEY_DOOR) {     //which vault does this key open?
        sprintf(opensVaultNumber, csvOutput ? "%i" : " (opens vault %i)", pmap[theItem->keyLoc[0].x][theItem->keyLoc[0].y].machineNumber - 1);
    }

    if (dropNumber > 0) {
        sprintf(monsterDropNumber, csvOutput ? "%i" : " (%i)", dropNumber);
        depth = 0;
    } else {
        depth = rogue.depthLevel;
    }

    if (csvOutput) {
        printSeedCatalogCsvLine(rogue.seed, depth, theItem->quantity, categoryName, kindName, enchantment, runicName, 
                                vaultNumber, opensVaultNumber, carriedByMonsterName, monsterDropNumber);
    } else {
        upperCase(inGameItemName);
        printf("        %s%s%s%s%s\n", inGameItemName, carriedByMonsterName, vaultNumber, opensVaultNumber, monsterDropNumber);
    }
}

void printSeedCatalogFloorItems(boolean csvOutput) {
    item *theItem;
    int gold = 0;
    short piles = 0;

    for (theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (theItem->category == GOLD) {
            piles++;
            gold += theItem->quantity;
        } else if (theItem->category == AMULET) {
        } else {
            printSeedCatalogItem(theItem, NULL, csvOutput, 0);
        }
    }
    if (gold > 0) {
        printSeedCatalogFloorGold(gold, piles, csvOutput);
    }
}

void printSeedCatalogMonsterItemsHopper(short maxItems, boolean csvOutput) {
    item *theItem;
    short dropNumber = 1, itemsCount = 1;

    for (theItem = monsterItemsHopper->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (itemsCount > maxItems) {
            return;
        }
        if (theItem->category != GOLD) {
            printSeedCatalogItem(theItem, NULL, csvOutput, dropNumber);
            itemsCount++;
        }
        dropNumber++;
    }
}

void printSeedCatalogMonster(creature *theMonster, boolean includeAll, boolean csvOutput) {
    char categoryName[10] = "", descriptor[15] = "", mutationName[15] = "", kindName[50] = "";

    if (!(includeAll || theMonster->bookkeepingFlags & MB_CAPTIVE || theMonster->creatureState == MONSTER_ALLY)) {
        return; // do nothing if the monster isn't an ally and we're not printing all monsters
    }

    if (theMonster->mutationIndex >= 0) {
       sprintf(mutationName, " (%s)", mutationCatalog[theMonster->mutationIndex].title);
    }

    if (theMonster->bookkeepingFlags & MB_CAPTIVE) {
        strcpy(categoryName,"ally");
        if (cellHasTMFlag(theMonster->xLoc, theMonster->yLoc, TM_PROMOTES_WITH_KEY)) {
            strcpy(descriptor, csvOutput ? " [caged]" : "A caged ");
        } else {
            strcpy(descriptor, csvOutput ? " [shackled]" : "A shackled ");
        }
    } else if (theMonster->creatureState == MONSTER_ALLY) {
        strcpy(categoryName,"ally");
        strcpy(descriptor, csvOutput ? "" : "An allied ");
    } else {
        strcpy(categoryName,"monster");
    }

    if (csvOutput) {
        sprintf(kindName, "%s%s%s",theMonster->info.monsterName, descriptor, mutationName);
        printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, 1, categoryName, kindName, "", "", "", "", "", "");
    } else {
        printf("        %s%s%s\n", descriptor, theMonster->info.monsterName, mutationName);
    }
}

void printSeedCatalogMonsters(boolean includeAll, boolean csvOutput) {
    creature *theMonster;

    for (theMonster = monsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        printSeedCatalogMonster(theMonster, includeAll, csvOutput);
    }

    for (theMonster = dormantMonsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        printSeedCatalogMonster(theMonster, includeAll, csvOutput);
    }
}

void printSeedCatalogMonsterItems(boolean csvOutput) {
    creature *theMonster;

    for (theMonster = monsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        if (theMonster->carriedItem != NULL && theMonster->carriedItem->category != GOLD) {
            printSeedCatalogItem(theMonster->carriedItem, theMonster, csvOutput, -1);
        }
    }

    for (theMonster = dormantMonsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        if (theMonster->carriedItem != NULL && theMonster->carriedItem->category != GOLD) {
            printSeedCatalogItem(theMonster->carriedItem, theMonster, csvOutput, -1);
        }
    }
}

void printSeedCatalogAltars(boolean csvOutput) {
    short i, j;
    boolean c_altars[50] = {0}; //IO.displayMachines uses 50
    char vaultNumber[10] = "";

    for (j = 0; j < DROWS; j++) {
        for (i = 0; i < DCOLS; i++) {
            if (pmap[i][j].layers[0] == RESURRECTION_ALTAR) {
                sprintf(vaultNumber, "%i", pmap[i][j].machineNumber);
                if (csvOutput) {
                    printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, 1, "altar", "resurrection altar", "", "", vaultNumber, "", "", "");
                } else {
                    printf("        A resurrection altar (vault %s)\n", vaultNumber);
                }
            }
            // commutation altars come in pairs. we only want to print 1.
            if (pmap[i][j].layers[0] == COMMUTATION_ALTAR) {
                c_altars[pmap[i][j].machineNumber] = true;
            }
        }
    }
    for (i = 0; i < 50; i++) {
        if (c_altars[i]) {
            sprintf(vaultNumber, "%i", i);
            if (csvOutput) {
                printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, 1, "altar", "commutation altar", "", "", vaultNumber, "", "", "");
            } else {
                printf("        A commutation altar (vault %s)\n",vaultNumber);
            }
        }
    }
}

void printSeedCatalog(unsigned long startingSeed, unsigned int numberOfSeedsToScan, unsigned int scanThroughDepth,
                      boolean includeAllMonsters, unsigned int maxMonsterDrops, boolean csvOutput) {
    unsigned long theSeed;
    unsigned int maxAllowedDepth;
    char path[BROGUE_FILENAME_MAX];
    char message[1000] = "";
    rogue.nextGame = NG_NOTHING;

    getAvailableFilePath(path, LAST_GAME_NAME, GAME_SUFFIX);
    strcat(path, GAME_SUFFIX);

    maxAllowedDepth = min(scanThroughDepth, DEEPEST_LEVEL);
    fprintf(stderr, "Scanning %u seed(s), %u level(s) per seed, from seed %lu.\n", numberOfSeedsToScan, maxAllowedDepth, startingSeed);

    sprintf(message, "Brogue seed catalog, seeds %li to %li, through depth %i.\n"
                     "Generated with %s. Dungeons unchanged since %s.\n\n"
                     "To play one of these seeds, press control-N from the title screen"
                     "and enter the seed number. Knowing which items will appear on"
                     "the first %i depths will, of course, make the game significantly easier.\n",
            startingSeed, startingSeed + numberOfSeedsToScan - 1, maxAllowedDepth, BROGUE_VERSION_STRING, BROGUE_DUNGEON_VERSION_STRING, maxAllowedDepth);

    if (csvOutput) {
        fprintf(stderr, "%s", message);
        printf("dungeon_version,seed,depth,quantity,category,kind,enchantment,runic,vault_number,opens_vault_number,carried_by_monster_name,monster_drop_number\n");
    } else {
        printf("%s", message);
    }

    for (theSeed = startingSeed; theSeed < startingSeed + numberOfSeedsToScan; theSeed++) {
        if (!csvOutput) {
            printf("Seed %li:\n", theSeed);
        }
        fprintf(stderr, "Scanning seed %li...\n", theSeed);
        rogue.nextGamePath[0] = '\0';
        randomNumbersGenerated = 0;

        rogue.playbackMode = false;
        rogue.playbackFastForward = false;
        rogue.playbackBetweenTurns = false;

        strcpy(currentFilePath, path);
        initializeRogue(theSeed);
        rogue.playbackOmniscience = true;
        for (rogue.depthLevel = 1; rogue.depthLevel <= scanThroughDepth; rogue.depthLevel++) {
            startLevel(rogue.depthLevel == 1 ? 1 : rogue.depthLevel - 1, 1); // descending into level n
            if (!csvOutput) {
                printf("    Depth %i:\n", rogue.depthLevel);
            }
            printSeedCatalogFloorItems(csvOutput);
            printSeedCatalogMonsterItems(csvOutput);
            printSeedCatalogMonsters(false, csvOutput); // captives and allies only
            if (rogue.depthLevel >= 13) { // resurrection & commutation altars can spawn starting on 13
                printSeedCatalogAltars(csvOutput);
            }
        }

        if (maxMonsterDrops > 0) {
            if (!csvOutput) {
                printf("    Monster Drops:\n");
            }
            printSeedCatalogMonsterItemsHopper(maxMonsterDrops, csvOutput);
        }

        freeEverything();
        remove(currentFilePath); // Don't add a spurious LastGame file to the brogue folder.
    }

}