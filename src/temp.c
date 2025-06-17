

//create the scene, limited by the macro N_OBJECTS
void create_scene()
{
  int i=0;
  //Let there be light:
  Object light              = {{0}};
  light.start_coordinate    = (Vector){0, 0, 500};
  light.type                = LIGHT;
  light.radius              = 35; //for render
  scene[i]                  = light;
  i++;
  //setting up floor:
  Object plane              = {{0}};
  plane.start_coordinate    = (Vector){100, 0, 950};
  plane.end_coordinate      = (Vector){0, 0, 0};
  plane.material.color      = (Color){0, 0.2, 0};
  plane.material.reflection = 0.1;
  plane.type                = PLANE;
  plane.radius              = 0; //N/A
  scene[i]                  = plane;
  i++;
  //  {x,y,z}, xy plane is parallel with screen
  
  int offset = 170;
  int scale  = 80;

  for(double theta=0; theta<=2*pi; theta+=2.0*pi/6)
    {   

      Object tmp              = {{0}};
      tmp.start_coordinate    = (Vector){offset + scale*cos(theta), offset + scale*sin(theta), 500};
      tmp.type                = SPHERE;
      tmp.radius              = 35;  
      tmp.material.reflection = 0.3;
     
      tmp.material.color = (Color){fabs(cos(theta)), fabs(sin(theta)), 0.5*theta/pi};

      scene[i]                = tmp;
      i++;
    }
  
printf("scene created...\n\n");
}
