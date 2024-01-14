#pragma once

// raw cube mesh containing position, normal, and texture coordinate vectors
static float CUBE_MESH[] =
{
   // Back face             normals               texture cords
    -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // Bottom-left
     0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f, // bottom-right
     0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // top-right
    -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // bottom-left
    -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 1.0f, // top-left
    // Front face
    -0.5f, -0.5f,  0.5f,    0.0f,  0.0f, 1.0f,    0.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f,    0.0f,  0.0f, 1.0f,    1.0f, 0.0f, // bottom-right
     0.5f,  0.5f,  0.5f,    0.0f,  0.0f, 1.0f,    1.0f, 1.0f, // top-right
     0.5f,  0.5f,  0.5f,    0.0f,  0.0f, 1.0f,    1.0f, 1.0f, // top-right
    -0.5f,  0.5f,  0.5f,    0.0f,  0.0f, 1.0f,    0.0f, 1.0f, // top-left
    -0.5f, -0.5f,  0.5f,    0.0f,  0.0f, 1.0f,    0.0f, 0.0f, // bottom-left
    // Left face
    -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
    -0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-left
    -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-right
    -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
    // Right face
     0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // top-left
     0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // bottom-right
     0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // bottom-right
     0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // top-left
     0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // bottom-left
    // Bottom face
    -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // top-left
     0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
    -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // bottom-right
    -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
    // Top face
    -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
     0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
     0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // top-right
     0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
    -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
    -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,  0.0f, 0.0f  // bottom-left
};

// quad mesh mainly used for the screen plane or drawing images
static float QUAD_MESH[] =
 {
 	// positions   // tex coords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

// a cube that only contains position vectors mostly used for skyboxes
static float CUBE_VERTS[] =
 {
 -1.0f,  1.0f, -1.0f,
 -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f,  1.0f, -1.0f,
 -1.0f,  1.0f, -1.0f,

 -1.0f, -1.0f,  1.0f,
 -1.0f, -1.0f, -1.0f,
 -1.0f,  1.0f, -1.0f,
 -1.0f,  1.0f, -1.0f,
 -1.0f,  1.0f,  1.0f,
 -1.0f, -1.0f,  1.0f,

  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,

 -1.0f, -1.0f,  1.0f,
 -1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f, -1.0f,  1.0f,
 -1.0f, -1.0f,  1.0f,

 -1.0f,  1.0f, -1.0f,
  1.0f,  1.0f, -1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
 -1.0f,  1.0f,  1.0f,
 -1.0f,  1.0f, -1.0f,

 -1.0f, -1.0f, -1.0f,
 -1.0f, -1.0f,  1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
 -1.0f, -1.0f,  1.0f,
  1.0f, -1.0f,  1.0f
};

