# Napo-AI-on

In this project we explore various agents for the Cyborg Uprising/Ghost in the cell game (https://www.codingame.com/ide/puzzle/ghost-in-the-cell).

## Agents

- Random
Takes list of all possible actions in a given position and selects a random subset of them. It should serve as baseline and almost always lose to other agents.
- Basic
Ignores bombs and increases. For each of our factories searches for first enemy factory we can conquer and sends troops there. It is brutally simple, but this bot was able to get to *bronze league* and wins more games than it loses with Flat_MC bot, so it is included here.
- Heuristic1 
Combination of many heuristics and small improvements. Doesn't include any big algorithm or opponent move prediction. It was able to get to *silver league* and place there near the middle, around 450th place.
- Flat_MC
Flat Monte Carlo implementation. In this game for each move we have many possibilities and we can select not just one, but many of them. Therefore it is hard to simulate using MC approach, as trees are very wide and different moves' statistics can interfere with each other. This model is decent, gets about 700th in silver league, but it is worse than Heuristic1 and about on par with Basic.
