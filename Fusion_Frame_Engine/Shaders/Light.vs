 #version 460 core

 layout(location = 0) in vec3 vertexdata;

 out vec3 CurrentPos;

 uniform mat4 model;
 uniform mat4 ProjView;

 void main()
 {
   CurrentPos = vec3(model * vec4(vertexdata,1.0f));
   gl_Position = ProjView * vec4(CurrentPos,1.0f);
 }
