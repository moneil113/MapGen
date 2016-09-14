# MapGen

Procedurally generate stylized maps
---
## Installation

Clone or download the repository, then run `make` in the root directory.

## Use

Run `MapGen` to use the default settings, or run `MapGen -c` to alter settings.

---

## Examples

![Map 1](Examples/Map1.png "Default settings")
![Map 2](Examples/Map2.png "High scale value")
![Map 3](Examples/Map3.png "Random seed")
![Map 4](Examples/Map4.png "High lacunarity")

---

## Progress Pictures

Raw open simplex noise height map
![Raw](ProgressImages/HeightMap.png)

Height map with values below a threshold cut off
![Falloff](ProgressImages/FalloffMap.png)

Islands colored to find distinct landmasses
![Colored](ProgressImages/ColoredIslands.png)

Islands outlined around their edges
![Colored](ProgressImages/IslandOutlines.png)

Final product
![Final](ProgressImages/FinalMap.png)
