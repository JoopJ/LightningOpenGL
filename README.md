# LightningOpenGL
An exploration of the methods used to create realistic looking and behaving lightning.

## Uses
- glad-4.6
- glfw-3.3.8
- glm-0.9.9.8
- imgui-1.89.3
- stb_image-2.28
- irrlicht-1.8.5

## How to use

Space - Recalculates the lightning pattern.

W/A/S/D - Move Camera

Mouse - Rotate Camera angle

M - Toggle Mouse camera movement

B - Toggle Lightning Bolt Glow

V - Toggle Shadows

Esc - Exit

## GUI
Methods: changes the method used to render the lightning bolt

Line - uses colored Line primitives

Triangle Color - uses coloured triangle primitives

### TODO:

- Shadow Mapping:
	- Multiple Lights:
		- Store information needed for object shader together: Depth Cube map, Position. (limited space)
		- Pass other information to Object Shader (diffuse, attenuation, etc...)
		- Replace current Point Lights

- De-clutter Main:
	- Move GUI code to separate file
		- Might have issues passing values
	- Move Buffer and Texture Object generation to separate file
	- Update and move Bolt definition and rendering to separate file
	
- L-System

- Branching

- Blur Optimization

- Deferred Rendering for large number of lights from bolt
