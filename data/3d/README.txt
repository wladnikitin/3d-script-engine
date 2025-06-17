This folder contains a file named cube.json, which describes a 3D model in a structured JSON format.

The structure is as follows:

{
  "modelName": "string",             // The name of the model
  "castShadow": true/false,         // Whether the model casts shadows
  "polygons": [                     // A list of polygon groups
    {
      "roughness": float,           // Surface roughness (0.0–1.0)
      "metallic": float,            // Surface metallic factor (0.0–1.0)
      "lightTarget": "string",      // Lighting target info (e.g. "none")
      "lightType": "string",        // Lighting type (e.g. "normal")
      "lines": [                    // List of lines (wireframe or edges)
        {
          "points": [               // A line is defined by 1 or more points
            {
              "x": float,           // X position in centimeters
              "y": float,           // Y position in centimeters
              "z": float,           // Z position in centimeters
              "r": 0–255,           // Red color channel
              "g": 0–255,           // Green color channel
              "b": 0–255,           // Blue color channel
              "opacity": 0.0–1.0,   // Transparency (1.0 = solid)
              "lightIntensity": 0.0–1.0 // Light emission from the point
            }
          ]
        }
      ]
    }
  ]
}


Meaning:

The modelName is used for identification and debugging.

Each polygon block represents a group of lines and their visual/material properties.

Each line contains one or more 3D points that form edges or corners.

Each point contains both geometric and visual information (position, color, opacity, light).