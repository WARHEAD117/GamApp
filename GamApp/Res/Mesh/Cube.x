xof 0303txt 0032
template Frame {
 <3d82ab46-62da-11cf-ab39-0020af71e433>
 [...]
}

template Matrix4x4 {
 <f6f23f45-7686-11cf-8f52-0040333594a3>
 array FLOAT matrix[16];
}

template FrameTransformMatrix {
 <f6f23f41-7686-11cf-8f52-0040333594a3>
 Matrix4x4 frameMatrix;
}

template Vector {
 <3d82ab5e-62da-11cf-ab39-0020af71e433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template MeshFace {
 <3d82ab5f-62da-11cf-ab39-0020af71e433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template Mesh {
 <3d82ab44-62da-11cf-ab39-0020af71e433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

template ColorRGBA {
 <35ff44e0-6c7c-11cf-8f52-0040333594a3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <d3e16e81-7835-11cf-8f52-0040333594a3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template Material {
 <3d82ab4d-62da-11cf-ab39-0020af71e433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template MeshMaterialList {
 <f6f23f42-7686-11cf-8f52-0040333594a3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material <3d82ab4d-62da-11cf-ab39-0020af71e433>]
}

template VertexElement {
 <f752461c-1e23-48f6-b9f8-8350850f336f>
 DWORD Type;
 DWORD Method;
 DWORD Usage;
 DWORD UsageIndex;
}

template DeclData {
 <bf22e553-292c-4781-9fea-62bd554bdd93>
 DWORD nElements;
 array VertexElement Elements[nElements];
 DWORD nDWords;
 array DWORD data[nDWords];
}

template AnimTicksPerSecond {
 <9e415a43-7ba6-4a73-8743-b73d47e88476>
 DWORD AnimTicksPerSecond;
}

template Animation {
 <3d82ab4f-62da-11cf-ab39-0020af71e433>
 [...]
}

template AnimationSet {
 <3d82ab50-62da-11cf-ab39-0020af71e433>
 [Animation <3d82ab4f-62da-11cf-ab39-0020af71e433>]
}

template FloatKeys {
 <10dd46a9-775b-11cf-8f52-0040333594a3>
 DWORD nValues;
 array FLOAT values[nValues];
}

template TimedFloatKeys {
 <f406b180-7b3b-11cf-8f52-0040333594a3>
 DWORD time;
 FloatKeys tfkeys;
}

template AnimationKey {
 <10dd46a8-775b-11cf-8f52-0040333594a3>
 DWORD keyType;
 DWORD nKeys;
 array TimedFloatKeys keys[nKeys];
}


Frame Box001 {
 

 FrameTransformMatrix {
  1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
 }

 Mesh Box001 {
  24;
  -0.500000;0.000000;-0.500000;,
  0.500000;0.000000;0.500000;,
  -0.500000;0.000000;0.500000;,
  0.500000;0.000000;-0.500000;,
  -0.500000;1.000000;-0.500000;,
  0.500000;1.000000;0.500000;,
  0.500000;1.000000;-0.500000;,
  -0.500000;1.000000;0.500000;,
  -0.500000;0.000000;-0.500000;,
  0.500000;1.000000;-0.500000;,
  0.500000;0.000000;-0.500000;,
  -0.500000;1.000000;-0.500000;,
  0.500000;0.000000;-0.500000;,
  0.500000;1.000000;0.500000;,
  0.500000;0.000000;0.500000;,
  0.500000;1.000000;-0.500000;,
  0.500000;0.000000;0.500000;,
  -0.500000;1.000000;0.500000;,
  -0.500000;0.000000;0.500000;,
  0.500000;1.000000;0.500000;,
  -0.500000;0.000000;0.500000;,
  -0.500000;1.000000;-0.500000;,
  -0.500000;0.000000;-0.500000;,
  -0.500000;1.000000;0.500000;;
  12;
  3;0,1,2;,
  3;1,0,3;,
  3;4,5,6;,
  3;5,4,7;,
  3;8,9,10;,
  3;9,8,11;,
  3;12,13,14;,
  3;13,12,15;,
  3;16,17,18;,
  3;17,16,19;,
  3;20,21,22;,
  3;21,20,23;;

  MeshMaterialList {
   1;
   12;
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0;

   Material {
    0.031373;0.239216;0.541177;1.000000;;
    6.000000;
    0.200000;0.200000;0.200000;;
    0.000000;0.000000;0.000000;;
   }
  }

  DeclData {
   1;
   2;0;3;0;;
   72;
   0,
   3212836864,
   0,
   0,
   3212836864,
   0,
   0,
   3212836864,
   0,
   0,
   3212836864,
   0,
   0,
   1065353216,
   0,
   0,
   1065353216,
   0,
   0,
   1065353216,
   0,
   0,
   1065353216,
   0,
   0,
   0,
   3212836864,
   0,
   0,
   3212836864,
   0,
   0,
   3212836864,
   0,
   0,
   3212836864,
   1065353216,
   0,
   0,
   1065353216,
   0,
   0,
   1065353216,
   0,
   0,
   1065353216,
   0,
   0,
   0,
   0,
   1065353216,
   0,
   0,
   1065353216,
   0,
   0,
   1065353216,
   0,
   0,
   1065353216,
   3212836864,
   0,
   0,
   3212836864,
   0,
   0,
   3212836864,
   0,
   0,
   3212836864,
   0,
   0;
  }
 }
}

AnimTicksPerSecond  {
 4800;
}

AnimationSet Anim1 {
 

 Animation {
  
  { Box001 }

  AnimationKey R {
   0;
   2;
   0;4;1.000000,0.000000,0.000000,0.000000;;,
   16000;4;1.000000,0.000000,0.000000,0.000000;;;
  }

  AnimationKey S {
   1;
   2;
   0;3;1.000000,1.000000,1.000000;;,
   16000;3;1.000000,1.000000,1.000000;;;
  }

  AnimationKey T {
   2;
   2;
   0;3;0.000000,0.000000,0.000000;;,
   16000;3;0.000000,0.000000,0.000000;;;
  }
 }
}