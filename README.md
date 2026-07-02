# Random Falloff tool for Modo plug-in

<b>Random Falloff</b> is a subtool that sets a random falloff value for mesh elements.<br>

<b>Random Falloff</b> provides three source types to generate random falloff weight values. <b>Element</b> generate falloff weights for each mesh vertex, edge, and polygon. Mesh Elements depend on the currently selected selection mode, generating falloff weights for the selected mesh elements. For vertices shared by edges and polygons, the weights of the vertices constituting the previously set element are used. <b>Selection Island</b> sets falloff weights for consecutive selection units. If nothing is selected, falloff weights are generated for connected polygon groups. <b>Part Tag</b> generate falloff weights for polygon groups with the same part polygon tag.

<b>Seed</b> is a random seed for generating random values.

When <b>Bipolar</b> is enabled, falloff weights are generated in the range of -1.0 to 1.0. When it's off, weights are generated in the range of 0.0 to 1.0.<br><b>

This kit contains a direct modeling tool for Modo macOS and Windows.

<div align="left">
<img src="images/vertex.gif" style='max-height: 500px; object-fit: contain'/>
</div>
<div align="left">
<img src="images/edge.gif" style='max-height: 500px; object-fit: contain'/>
</div>

## Installing

- Download lpk from releases. Drag and drop it into your Modo viewport. If you're upgrading, delete previous version.

## How to use Random Falloff tool

- The tool version of Random Falloff can be launched from **Falloff** popup menu on the menu bar.

<div align="left">
<img src="images/UI.png" style='max-height: 620px; object-fit: contain'/>
</div>

## Basic Usage with Random Falloff tool

- Choose **Random** menu on Faloff popup menu.
- Active an actor tool such as transform tool.

