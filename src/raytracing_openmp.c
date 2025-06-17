/*
Matz OpenMP-trådade ray-tracer, sporadicly programmed 19-23 Dec 2010

branched from raytracing.c [23i12~10]

Obsolete:::To define a sphere we use start_coordinate as center and
end_coordinate as outer limit (distance between them as radius).
We now use radius, to make things simpler


TODO:
Add...
* render lights (intersection test against lights)
* phong shading
* light attenuation
* clamp colors or use in hdri, the effect is easily seen on reflective surfaces >0.5

News:

* [27i12~10] Added color to ray

* [26i12~10] Fixed bug with sphere intersection code
   Added plane as an object
   Added functionality (get_normal, trace_ray,...)
   It now finds intersection for the closest object

*/

//use OpenMP to "thread" the render process

//#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>


#include "GSDL.h"
#include "bmpimage.h"
#include <omp.h>

//#include <sys/types.h>
#include <sys/time.h>//must have this


typedef struct _Vector {
  double x,y,z;
} Vector;

typedef struct _Color {
  double r,g,b;//<-[0,1]
} Color;

typedef struct _Ray {
  Vector origin;
  Vector direction;
} Ray;


//the material properties:
typedef struct _Material {
  double reflection;//[0,1] 0 means no reflection
  double refraction;
  Color color;
} Material;

//object definition:
typedef struct _Object {
  Vector start_coordinate;//for sphere this is the same as "center"
  Vector end_coordinate; //is not applicable for: light
  Vector orientation;    //orientation, N/A for sphere
  Material material;
  
  double radius;//undefined for non-sphere, then start/end-coordinate is used
  int type;//type of object
} Object;




//type of objects:
//note: we don't define 0 since this will match NULL-initiated objects
#define LIGHT  1
#define PLANE  2
#define CUBE   3
#define SPHERE 4

#define N_OBJECTS 6+2 //number of object including lights
#define EPS 0.00001 //offset for ray tracing (prevents acne)
#define in(a, size, i, j) *(a + i*size + j) //indexing array

//OpenMP:
#define DEBUG 1 //printing debug information
#define N_THREADS 2


double pi = 3.141592653589793;
double ambient_light = 0.2;//the light where the light doesn't reach

Object scene[N_OBJECTS];

//OpenMP:
int threads_workload[N_THREADS];

/*The scene with objects, for now only one light is supported and it
  is at first index in the scene array, later on this will probably be a
  spatial tree
*/
Color background_color = {0.1, 0.1, 0.1};


//////////////////////////////////////////////////////////////////////

void create_scene();
double vec_distance(Vector *point1, Vector *point2);

Color cols_mult(Color *a, double m);
Color col_add(Color *a, Color *b);
Color col_normalize(Color *a); //clamping the color
Color cols_add(Color *a, double s);

Color get_point_color(Object *obj, Vector *coordinate);

Vector is_intersecting_object(Ray *t, Object *);

double vec_dot(Vector *a, Vector *b);
Vector vec_add(Vector *a, Vector *b);
Vector vec_sub(Vector *a, Vector *b);
int vec_isnull(Vector *a);

Vector vecs_mult(Vector *a, double s);
Vector vecs_add(Vector *v, double s);
double vec_len(Vector *a);

Vector get_normal(Object *obj, Vector *coordinate);


void render(Color *buffer, int size, int rectw, double FOV, int silent);
void buffer2BMP(Color *buffer, int size, int rectw);
void print_render(Color *buffer, int size, int rectw);


void create_scene();
Ray get_reflected_ray(Ray *original_ray, Object *obj);

Color trace_ray(Ray *original_ray, int level);

int ray_intersection_test(Ray *t, int light);

double csecond();

///////////////////////////////////

/*
double csecond()
{
  struct timeval  tv;
  gettimeofday(&tv, 0);
  //printf("%f\n", tv.tv_sec);
  return tv.tv_sec + tv.tv_usec * 1.0e-6;
  }*/

//dot product:
double vec_dot(Vector *a, Vector *b)
{
  return a->x*b->x + a->y*b->y + a->z*b->z;
}


//vector scalar addition:
Vector vecs_add(Vector *v, double s)
{
  return (Vector){v->x + s, v->y + s, v->z + s};
}


Vector vec_add(Vector *a, Vector *b)
{
  return (Vector){a->x + b->x, a->y + b->y, a->z + b->z};
  
}

Vector vec_sub(Vector *a, Vector *b)
{
  return (Vector){a->x - b->x, a->y - b->y, a->z - b->z};
}

double vec_distance(Vector *a, Vector *b)
{
  double dist2 =  pow(a->x - b->x, 2.0) + 
    pow(a->y - b->y, 2.0) + 
    pow(a->z - b->z, 2.0);
  
  return sqrt(dist2);
}

Vector vec_normalize(Vector *a)
{
  //replace demon with distance(a, a)
  double denom = 1/vec_len(a);
  Vector b = (Vector){a->x*denom, a->y*denom, a->z*denom};
  return b;
}

double vec_len(Vector *a)
{
  return sqrt(a->x*a->x + a->y*a->y + a->z*a->z);
}


double vec_dist_squared(Vector *a, Vector *b)
{
  return pow(a->x - b->x, 2.0) + pow(a->y - b->y, 2.0) + pow(a->z - b->z, 2.0);
}


//returns the (nearest) normal of object <obj> at world coordinate <coordinate>
//works only for spheres atm [19i12~10]
//the objects' orientation should be calculated, used for: plane, box,...

Vector get_normal(Object *obj, Vector *coordinate)
{
  //testing so the coordinate is somewhat close to the object
  if(obj->type==SPHERE && (vec_distance(&obj->start_coordinate, coordinate) - obj->radius) < EPS )
    {
      Vector a = vec_sub(coordinate, &obj->start_coordinate);
      Vector b = vec_normalize(&a);
      return b;
    } 
  else if (obj->type==PLANE) //should test if close to surface?
    {
      Vector a = (Vector){1, 0, 1};//(1,0,1) 45 degrees 
      Vector b = vec_normalize(&a);      
      return b;
    }
  else
    {
      printf("  GET_NORMAL ==> the normal is not correct\n");
      return (Vector){1};
    }
}


//color scalar addition:
Color cols_add(Color *a, double s)
{
  return (Color){a->r+s, a->g+s, a->b+s};
}

//color scalar multiplication:
Color cols_mult(Color *a, double s)
{
  return (Color){a->r*s, a->g*s, a->b*s};
}


Color col_add(Color *a, Color *b)
{
  return (Color){a->r + b->r, a->g + b->g, a->b + b->b};
}

Color col_normalize(Color *a) //clamping the color
{
  return (Color){(a->r>1)?1:a->r, (a->g>1)?1:a->g, (a->b>1)?1:a->b};
}


///////////////////////////////////
int vec_isnull(Vector *a)
{
  return a->x==0 && a->y==0 && a->z==0;
}


//vector scalar multiplication
Vector vecs_mult(Vector *a, double s)
{
  return  (Vector){a->x*s, a->y*s, a->z*s};
}


//returns index to object if t intersects it.
int ray_intersection_test(Ray *t, int light)
{
  int intersect_index          = -1;
  double tmp_dist_squared_best = 10000000;//infinity
  double tmp_dist_squared;
  
  for(int i=0; i<N_OBJECTS; i++)
    {
      /*if the ray hits an object, then return that objects' index. If
	there are several objects that are hit, then return the
	closest one. If argument <light> is 1 then the test returns if
	an intersection occured, not caring about the closest
	intersection.
      */
      
      //      if((scene[i]).type==SPHERE)
      {
	Vector q = is_intersecting_object(t, &scene[i]);
	
	
	if (!vec_isnull(&q))
	  {
	    if(light)//used by shadow ray test
	      return i;
	    tmp_dist_squared = vec_dist_squared(&q, &t->origin);	      
	    //calc distance between t.origin and q    
	    if(tmp_dist_squared < tmp_dist_squared_best)
	      {
		tmp_dist_squared_best = tmp_dist_squared;//found a better one
		intersect_index = i;
	      }
	  }
      }
    }
  
  return intersect_index;//it doesn't intersect anything
}


/*returns the coordinate of the intersection of vector <Ray> with
  object <obj> 
*/
Vector is_intersecting_object(Ray *t, Object *obj)
{
  /*
    v = dot(EO, v)
    dist = r² - (dot(origin,origin) - v²)
  */
  Vector EO   = vec_sub(&obj->start_coordinate, &t->origin);
  double d; 
  double dist;
  Vector p = {0};
  double v = 0;
  
  if(obj->type==SPHERE)
    {
      
      double r2   = obj->radius*obj->radius;
      v    = vec_dot(&EO, &t->direction);
      double v2   = pow(v, 2.0);
      dist = r2 - ( vec_dot(&EO, &EO) - v2 );
    }
  
  else if(obj->type==PLANE)
    {
      /*for now, the normal is fixed and the plane is infinite*/
      Vector n = get_normal(obj, NULL);//we don't use arg. 2
      double numerator = vec_dot(&EO, &n);
      double denominator = vec_dot(&t->direction, &n);
      d = numerator/denominator-EPS;
      /*preventing acne, this should be negative pushing the ray in
	the direction of the ray itself*/
    }
  
  if(obj->type==SPHERE && dist>=0)//if <0 then no intersection
    {
      d = sqrt(dist)-EPS;// offset to prevent "acne"
      
      Vector tmp = vecs_mult(&t->direction, v-d);
      p = vec_add(&t->origin, &tmp);
      
      return p;   
    }
  else if(obj->type==PLANE && d>=0)
    {
      Vector tmp = vecs_mult(&t->direction, d);
      p = vec_add(&t->origin, &tmp);
      
      return p;
    }
    
  //  printf("is_intersecting_object:: The ray does not intersect object\n");
  return (Vector){0};
}


//returns the color of a ray that intersects object <obj>
Color get_point_color(Object *obj, Vector *coordinate)
{
  Object light = scene[0];//given: first element in array is a light
  
  Vector light_vector = vec_sub(&light.start_coordinate, coordinate);
  light_vector = vec_normalize(&light_vector);//needed?
  
  //testing to see if coordinate is in shadow or not:
  /*
  Ray shadow_ray = (Ray){0};
  shadow_ray.direction = light_vector;
  shadow_ray.origin = (Vector){coordinate->x, coordinate->y, coordinate->z};
  */
  
  /*light vector is the vector from the surface of the object the ray
    has intersected and the lightsource(s)*/
  Vector n = get_normal(obj, coordinate);
  double dark = 1;
  // int intersection_id = ray_intersection_test(&shadow_ray, 1);
  
  /*  if(intersection_id!=-1) //we know this point is in shadow
   dark=0.1;
  */
  double shade = vec_dot( &light_vector, &n);
  
  if(shade<0)//the ray intersected something else than light
    shade=0;
  
  /*trace the shadow ray toward the light and if anything is obscuring
    that ray, then the position is in the shadow
  */

  double scale = ambient_light + (1 - ambient_light) * shade*dark;
  Color point_color = cols_mult(&obj->material.color, scale);

  return point_color;
}


/*return the color of a ray*/
Color trace_ray(Ray *original_ray, int level)
{
  Color point_color, reflect_color, refract_color;
  int index = ray_intersection_test(original_ray, 0);
  
  if(index==-1)
    {
      // printf("Ray did not hit any object, returning gray\n");
      return background_color;//background
    }
  
  Object obj = scene[index];
  Vector coordinate = is_intersecting_object(original_ray, &obj);
  
  //if we trace right into a light
  if(obj.type==LIGHT)
    return (Color){1,1,1};//light color
  
  //Todo:  if object it intersects is a light, then return white.
  //let the white depend on distance from light coordinate

  point_color = get_point_color(&obj, &coordinate);
  
  
  if (obj.material.reflection>0 && level<=5)//limit number of recursive steps
    {
      Ray reflected = get_reflected_ray(original_ray, &obj);
      //add color to ray here?

      reflect_color = trace_ray(&reflected, level+1);
    }
  /*
  if (obj.material.refraction>0 && level<=7)
    {
      Ray refracted = get_refracted_ray(original_ray, &obj);
      refract_color = trace_ray(&refracted, level+1); 
    }
  */
  //add refraction later:
  double f = obj.material.reflection;
  //printf("obj.material.reflection:%f\n",obj.material.reflection);
  
  Color col1 = cols_mult(&reflect_color, f);
  Color col2 = cols_mult(&point_color, 1-f);
  
  //kolla så detta verkligen stämmer, det ser extremt starkt ut i rendering[27i12~10]
  Color col = col_add(&col1, &col2);
  return col;   
  //    return col_normalize(&col);
}


//calculate the reflection of ray <original_ray> on object <obj>
Ray get_reflected_ray(Ray *original_ray, Object *obj)
{
  Vector intersection = is_intersecting_object(original_ray, obj);
  Vector n = get_normal(obj, &intersection);
  
  //reflection:: c1 = -n.v; r = v+(2*n*c1)
  
  double c1   = -vec_dot(&n, &original_ray->direction);
  Vector npn   = vecs_mult(&n, 2.0);//n plus n
  Vector npnc1 = vecs_mult(&npn, c1);
  Vector reflected_vector = vec_add(&original_ray->direction, &npnc1);

  //return reflected ray, with new origin and direction
  Ray reflected_ray = {0};
  reflected_ray.origin  = intersection;//need to offset?
  reflected_ray.direction = reflected_vector;

  return reflected_ray;
}




/* render to matrix and then update screen */


void buffer2BMP(Color *buffer, int size, int rectw)
{
  int dim = size/rectw;    
  
  bit32 *bit32buffer;// = malloc(dim*dim*sizeof(bit32) + 1);
 
  if ((bit32buffer = (bit32*) calloc(dim*dim, sizeof(bit32))) == NULL) 
    {
      printf("buffer2BMP: Could not allocate buffer.\n");
      exit(1);
    }
  
  printf("saving bmp file (%dx%d)...\n", dim, dim);

  Color tmpcol;
  bit32 tmp32;

  for(int i=0; i<dim; i+=rectw)
    for(int j=0; j<dim; j+=rectw)
      {
	tmpcol = (Color) in(buffer, dim, i, j);
	
	//parenthesis are important below:
	tmp32 = (int)(255*tmpcol.r) + 
	  ((int)(255*tmpcol.g) << 8) +
	  ((int)(255*tmpcol.b) << 16);
	
	in(bit32buffer, dim, i, j) = tmp32;
      }
    
  SaveBMP32RGBA(bit32buffer, dim, dim, "test.bmp");
  //printf("bmp file has NOT been saved\n");
}


void print_render(Color *buffer, int size, int rectw)
{

  int dim = size/rectw;

  for (int i=0; i<dim; i++)
    for(int j=0; j<dim; j++)
      {
	Color color = in(buffer, dim, i, j);

	if(color.r>0 || color.g>0 || color.b>0)
	  printf("(%d,%d) --> @<%1.2f %1.2f %1.2f>\n", i, j, 
		 color.r, color.g, color.b);
      }
}

/*
  responsible for putting pixels on the screen, rectw is the "rectangle
  width",when the render puts pixels in the window it uses rectangles
  for the case rectw>1 to allow for fast rendering.

size - the (GUI) window size

FOV - Field of View, the angle between extreme right and left of the
"ray window". THe ray window is the grid in which the ray traces the
scene, it is an imaginary window and it along with the origin (0,0,0)
describes the rays and also the FOV.

 */
void render(Color *buffer, int rectw, int size, double FOV, int silent)
{
  if(size%rectw!=0)
    {
      printf("render: argument <res> must divide the size of the window\n");
      exit(-1);   
    }

  printf("Note: <Silent mode> has been disabled\n");
  printf("Setting pixel size:%d\n", rectw);
  printf("silent mode:%d\n", silent);
  /*
    To make things a little simpler I let the origin of the rays be on
    the z-axis and calculate the position the ray window will be
    placed given the FOV. The rays shoot with origo (0,0,0) as origin.
  */
  double tic = csecond();
  int dim = size/rectw;
  double angle = FOV/180*pi;//radians
  double d = (0.5*size)/sin(angle*0.5); 
//simple geometry, determines distance between screen and origin of the rays
  
  printf("Placing the 'ray window' to accomplish FOV %1.2f. at %1.1f\n", FOV, d);

  
  /*
  char title[100];
  sprintf(title, "Matz Raytracer [%s]", __DATE__);
  */
  //display.end_coordinate =(Vector){size, 0, size};
  
  printf("Initializing screen...\n");
 
  if( silent==0 )  
    init_screen(title, size, size);
  
  Color tmpcol;
  Vector vec;
  Ray scan_ray;//this is embarrasingly parallelizable

  printf(" Starting rendering...");
  fflush(stdout);   
  int x,i,z;//the loop variables must be outside of pragma
  
  if(DEBUG)
    for (i=0;i<N_THREADS;i++)
      threads_workload[i]=0;   
  
#pragma omp parallel default(none) private(x, scan_ray, vec, tmpcol) shared(buffer, threads_workload, size, dim, rectw, d)
  {
    //      i_am = omp_get_thread_num();	 
#pragma omp for private(z) schedule(dynamic)//j inte private 'by default'!
    //schedule(static,1)
    
    for(x=0; x<size; x+=rectw)
      {
	for(z=0; z<size; z+=rectw)
	  {
	    scan_ray = (Ray){{0}};
	    scan_ray.origin = (Vector){0, 0, 0};
	    
	    vec = (Vector){x, z, d};
	    scan_ray.direction = vec_normalize(&vec);
	    tmpcol = trace_ray(&scan_ray, 1);
	    //let the threads divide the scanning	  
	    if (DEBUG) threads_workload[omp_get_thread_num()]++;	  
	    in(buffer, dim, x/rectw, z/rectw) = tmpcol;
	  }
      }
  }
  printf("done\n");
  
  double toc = csecond();

  fprintf(stderr, "Elapsed time: %f s\n", toc-tic); 
  
  if (DEBUG)
    {
      double sum=0;
  
      for (i=0; i<N_THREADS; i++)
	sum+=threads_workload[i];
    
      printf("sum:%1.2f\n", sum);   
    
    
      for (i=0; i<N_THREADS; i++)
	printf("Thread #%i workload: %1.2f%%\n", i, 100.*(threads_workload[i]/sum));   
    }
  /*<eventloop> keeps the window open and polls for events, this is
    called after all pixels has been drawn*/
  
  if(silent==0)
    {  
      printf("Entering event polling loop...\n");
      eventloop();
    }   
    else

    
}


//create the scene, limited by the macro N_OBJECTS
//Todo: the objects should be read from file and here only placed/rotated into place in the world
void create_scene()
{
  int i=0;
  //Let there be light:

  Object light              = {{0}};
  light.start_coordinate    = (Vector){0, 500, 300};
  light.type                = LIGHT;
  light.radius              = 50; //for render
  scene[i]                  = light;
  i++;
  //setting up floor:

  Object plane              = {{0}};
  plane.start_coordinate    = (Vector){100, 0, 990};
  plane.end_coordinate      = (Vector){0, 0, 0};
  plane.material.color      = (Color){0, 0.2, 0};
  plane.material.reflection = 0.3;
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
      tmp.material.reflection = 0.2;
     
      tmp.material.color = (Color){fabs(cos(theta)), fabs(sin(theta)), 0.5*theta/pi};

      scene[i]                = tmp;
      i++;
    }
  
  printf("scene created...\n\n");
}


//Used to print all object in the scene
void print_scene()
{
    
  for(int i=0; i<N_OBJECTS; i++)
    {
      if(scene[i].type==SPHERE)
	printf("Object %d is a sphere, pos: (%1.1f %1.1f %1.1f), refl:%1.1f, radius %1.1f \n", 
	       i, 
	       scene[i].start_coordinate.x, 
	       scene[i].start_coordinate.y, 
	       scene[i].start_coordinate.z,
	       scene[i].material.reflection,
	       scene[i].radius);
      
      else if(scene[i].type==LIGHT)
	printf("Object %d is a light, pos: (%1.1f %1.1f %1.1f)\n", 
	       i, 
	       scene[i].start_coordinate.x, 
	       scene[i].start_coordinate.y, 
	       scene[i].start_coordinate.z);
      
      else if(scene[i].type==PLANE)
	printf("Object %d is a plane, pos: (%1.1f %1.1f %1.1f)\n", 
	       i, 
	       scene[i].start_coordinate.x, 
	       scene[i].start_coordinate.y, 
	       scene[i].start_coordinate.z);
    }

  printf("\n\n");
}

/*
int is_equal_colors(Color a, Color b)
{
  return fabs(a.r-b.r)<EPS && fabs(a.g-b.g)<EPS && fabs(a.b-b.b)<EPS;
}


void change_buffer(Color *buffer, int size, int rectw)
{
  int dim=size/rectw;
  
  for(int i=0; i<dim; i++)
    for(int j=0; j<dim; j++)
      in(buffer, dim, i, j) = (Color){1., 1., 1.};
  
  printf("");
  int flag=0;
  for(int i=0; i<dim; i++)
    for(int j=0; j<dim; j++)
      if(is_equal_colors(in(buffer, dim, i, j), (Color){1.,1.,1.}))
	flag++;
  
  if(flag)
    {
    printf("cannot change values in <buffer>, %1.1f%% of the elements were not white\n", 100*(double)(flag)/(dim*dim));
    exit(1);    
}  

}
*/

int main(void)
{

  printf("    *****                                  *****\n");
  printf("****************************************************\n");
  printf(" **************************************************\n");
  printf("   **                                          ** \n");
  printf("   **    Matz Ray Tracer           ver. 0.14   ** \n");
  printf("   **    Very limited beta        (26i12~10)   ** \n");
  printf("   **                                          ** \n");
  printf(" **************************************************\n");
  printf("****************************************************\n");
  printf("    *****                                  *****\n\n");
  
  printf("Setting up scene objects..."); 
  create_scene();
  print_scene();

  int size = 700;
  int rectw = 1;

  if(size%rectw!=0)
    {
      printf("size must be a multiple of rectw!\n");
      exit(-1);     
    }

  Color *buffer;//index with IN(buffer,i,j)
  int dim = size/rectw;

  if ((buffer = (Color*)calloc(dim*dim, sizeof(Color))) == NULL) 
    {
      printf("Could not malloc buffer.\n");
      exit(1);
    }
  
  /* Set no. of threads dynamically. */
  omp_set_num_threads(N_THREADS);
  
  /* Max number of procs and threads. */
  printf("num_procs   = %d\n", omp_get_num_procs());
  printf("max_threads = %d\n", omp_get_max_threads());
    

  render(buffer, rectw, size, 42, 0);
  buffer2BMP(buffer, size, rectw);

  //  print_render(buffer, size, rectw);
  printf("freeing buffer...\n");
  free(buffer);

  /*
    For some reason we must do a silent render when using remote4, the
    solution is to render to a buffer and then use that buffer to
    update the screen. This will hopefully be a faster way than to
    send each pixel via remote
  */


  return 0;
}




/*
  OLD stuff::
  
  Ray window:
  a:(   0, 10, size)
  b:(size, 10, size)
  c:(   0, 10, 0)
  d:(size, 10, 0)
  
  a______b
  |      |
  |      |
  |      |
  |______|
  c      d
  
  The ray window object has start coordinate (c) and end coordinate (b)
  The screen "origin" is at c, the lower left corner of the window.
  
*/
