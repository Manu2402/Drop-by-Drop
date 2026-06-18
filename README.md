# Drop by Drop

**Drop by Drop** is an Unreal Engine 5 editor plugin for procedural terrain generation and hydraulic erosion simulation. It integrates directly into the UE5 editor as a Slate-based tool panel, letting you generate or import a heightmap, spawn a landscape, split it into streaming proxies, and run a physically-based water erosion simulation — all without leaving the editor.

---

## Demo Video

[Watch the demo video](https://www.youtube.com/watch?v=33QFwI-PbNg&t=1s)

---

## Features

### Heightmap Generation
Generate a heightmap procedurally using multi-octave Perlin noise. All parameters are exposed and adjustable in the panel:

| Parameter | Default | Description |
|---|---|---|
| Seed | -4314 | RNG seed — determines the terrain pattern |
| Octaves | 8 | Number of noise layers combined |
| Persistence | 0.45 | Amplitude reduction per octave |
| Lacunarity | 2.0 | Frequency multiplier per octave |
| Initial Scale | 1.8 | Base frequency of the noise pattern |
| Max Height Difference | 1.0 | Normalization range for terrain height |
| Randomize Seed | false | Generates a new random seed on each run |

The output is a 505×505 float heightmap, saved as a `Texture2D` asset in the content browser.

### External Heightmap Import
Import an existing heightmap from the filesystem via a native file dialog. Supported pixel formats:

| Format | Description |
|---|---|
| G8 | 8-bit grayscale |
| G16 | 16-bit grayscale (high precision) |
| R32F | 32-bit floating point |
| RGBA8 | 8-bit RGBA |
| BGRA8 | 8-bit BGRA |
| RGBA16F | 16-bit half-float RGBA |

Imported heightmaps are resampled to 505×505 via bilinear interpolation. X, Y, Z scaling factors can be set independently.

### Landscape Generation
Spawn an `ALandscape` actor directly in the current level from the generated or imported heightmap. Landscape size is configurable in kilometers and the world partition cell size is adjustable for streaming.

### Landscape Proxies
Split the generated landscape into streaming proxies with a single click. Useful for large open-world levels that use World Partition.

### AutoMaterial
Automatically assign a landscape material after creation to enable layer blending and other material-driven features.

### Hydraulic Erosion Simulation
Simulate water erosion using a particle-based algorithm. Each virtual water droplet carries sediment, erodes terrain as it moves downhill, and deposits sediment when it slows down or evaporates.

**Erosion parameters:**

| Parameter | Default | Description |
|---|---|---|
| Erosion Cycles | 100,000 | Number of droplets simulated |
| Inertia | 0.30 | How much a droplet retains its current direction |
| Capacity | 8 | Maximum sediment a droplet can carry |
| Minimal Slope | 0.01 | Minimum slope required for erosion to occur |
| Deposition Speed | 0.20 | Rate at which sediment is deposited |
| Erosion Speed | 0.70 | Rate at which terrain is eroded |
| Gravity | 10 | Gravitational force on the droplet |
| Evaporation | 0.02 | Rate at which water volume decreases |
| Max Path | 64 | Maximum steps before a droplet terminates |
| Erosion Radius | 4 | Cell radius affected by each erosion/deposition event |

**Wind Bias:** optionally bias the droplet spawn position toward a compass direction to simulate directional weathering. Supported directions: Random, N, NE, E, SE, S, SW, W, NW.

A viewport arrow/cone preview shows the configured wind direction directly in the editor.

### Erosion Templates
Save and load erosion parameter sets as named presets. Templates are stored in a `UDataTable` (`FErosionTemplateRow`) and persist between editor sessions. The `UErosionTemplateManager` supports:
- Listing and searching all saved templates
- Saving the current erosion settings as a new template
- Loading a template into the active erosion settings
- Deleting templates

---

## Architecture

```
FDropByDropModule          — Plugin entry point, registers toolbar & commands
  └─ SRootPanel            — Top-level Slate widget, tab switching
       ├─ SHeightMapPanel  — Procedural + External heightmap Controls
       ├─ SLandscapePanel  — Landscape creation, proxies
       ├─ SErosionPanel    — Erosion parameters, wind bias compass
       └─ STemplateBrowser — Template list, Save/Load/Delete UI

UPipelineLibrary           — Static functions: Heightmap generation, Landscape spawn, Erosion, Templates
UErosionLibrary            — Core erosion algorithm ("FDrop" particle simulation)
UErosionTemplateManager    — DataTable-backed template CRUD
FDropByDropSettings        — Singleton: shared state and DataTable reference
ULandscapeInfoComponent    — ActorComponent attached to ALandscape, tracks erosion/proxy state
```

---

## Requirements

- Unreal Engine **5.6**
- Editor-only plugin (Type: `Editor`, LoadingPhase: `Default`)

---

## Installation

1. Copy the `DropByDrop` folder into your project's `Plugins/` directory.
2. Regenerate project files.
3. Build and open the project — the plugin loads automatically.
4. Access the tool from the editor toolbar button added by the plugin.

---

## Development

Developed by **Manuel Solano** and **Roberto Capparelli** as part of a research initiative on landscape generation tooling for Unreal Engine.
Special thanks to **Eugenio Fattori** for guidance during development.
