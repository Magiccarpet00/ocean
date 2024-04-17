/*!\file window.c
 *
 * \brief GL4Dummies, éclairage et modélisation
 * 
 * \author Farès BELHADJ,
 * amsi@up8.edu \date March 27 2024
 */

#include <GL4D/gl4dg.h>
#include <GL4D/gl4df.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4duw_SDL2.h>
#include <GL4D/gl4dp.h>


//--------------------
typedef struct
{
    float z;
    float selfTime;
    int go;
}vec3_t;

#define TENSOR_SIZE 100
#define TENSOR_DEPTH 5
#define KERNEL_SIZE 3
#define PI 3.14159265

static vec3_t tensor[TENSOR_SIZE][TENSOR_SIZE][TENSOR_DEPTH];
static vec3_t tmp_tensor[TENSOR_SIZE][TENSOR_SIZE][TENSOR_DEPTH];
static int nbWave = 0;

static float kernel[KERNEL_SIZE][KERNEL_SIZE] = {
    {0.2, 0.4, 0.2},
    {0.4, 0.0, 0.4},
    {0.2, 0.4, 0.2},
};
//--------------------


/* Prototypes des fonctions statiques contenues dans ce fichier C */
static void init(void);
static void keyd(int keycode);
static void draw(void);
static void quit(void);

static void addPerturbation(int x, int y, float vel_z);
static void convolution(void);
static void writeTex();

/*!\brief largeur et hauteur de la fenêtre */
// static int _wW = 1280, _wH = 960;
static int _wW = 1920, _wH = 1080;
static int _move = 10;
/*!\brief identifiant du (futur) GLSL program */
static GLuint _pId = 0;
/*!\brief identifiant pour une géométrie GL4D */
static GLuint _gridId = 0;
static GLuint _grid2Id = 1;
static GLuint _blur = 0;

/*!\brief identifiant pour une géométrie GL4D */
static GLboolean _wireframe = GL_FALSE;
/*!\brief identifiant de texture */
static GLuint _texId = 0;

static int _tw = 100, _th = 100;
// static int _tw = 513, _th = 513;

static GLfloat _eyeX = 0.0f;
static GLfloat _eyeY = 50.0f;
static GLfloat _eyeZ = -1500.0f;

static GLfloat _centerX = 0.0f;
static GLfloat _centerY = 0.0f;
static GLfloat _centerZ = 0.0f;


// const GLuint B = RGB(255, 255, 255), N = 0;
const GLuint B = 255, N = 0, G = 125;

GLuint tex[TENSOR_SIZE * TENSOR_SIZE];


/*!\brief créé la fenêtre d'affichage, initialise GL et les données,
 * affecte les fonctions d'événements et lance la boucle principale
 * d'affichage.
 */
int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "GL4Dummies", 20, 20, 
			 _wW, _wH, GL4DW_RESIZABLE | GL4DW_SHOWN))
    return 1;
  init();
  atexit(quit);
  gl4duwKeyDownFunc(keyd);
  gl4duwDisplayFunc(draw);
  gl4duwMainLoop();
  return 0;
}
/*!\brief initialise les paramètres OpenGL et les données. */
static void init(void) {
  glGenTextures(1, &_texId);
  glBindTexture(GL_TEXTURE_2D, _texId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  GLfloat * fractales = gl4dmTriangleEdge (_tw, _th, 0.6f);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _tw, _th, 0, GL_RED, GL_FLOAT, fractales);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TENSOR_SIZE, TENSOR_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
  
  free(fractales);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  /* Création du programme shader (voir le dossier shader) */
  _pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
  /* Créer un quadtrilatère */
  _gridId = gl4dgGenGrid2df(_tw, _th);
  // _grid2Id = gl4dgGenGrid2df(4,4);

  /* Set de la couleur (RGBA) d'effacement OpenGL */
  // rgb(135,206,235)
  glClearColor(0.53f, 0.80f, 0.92f, 1.0f);
  /* activation du test de profondeur afin de prendre en compte la
   * notion de devant-derrière. */
  glEnable(GL_DEPTH_TEST);

  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  /* Création des matrices GL4Dummies, une pour la projection, une
   * pour la modélisation et une pour la vue */
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  gl4duGenMatrix(GL_FLOAT, "modelMatrix");
  gl4duGenMatrix(GL_FLOAT, "viewMatrix");
  /* on active la matrice de projection créée précédemment */
  gl4duBindMatrix("projectionMatrix");
  /* la matrice en cours reçoit identité (matrice avec des 1 sur la
   * diagonale et que des 0 sur le reste) */
  gl4duLoadIdentityf();
  /* on multiplie la matrice en cours par une matrice de projection
   * orthographique ou perspective */
  /* décommenter pour orthographique gl4duOrthof(-1, 1, -1, 1, 0, 100); */
  gl4duFrustumf(-1.0f, 1.0f, (-1.0f * _wH) / _wW, (1.0f * _wH) / _wW, 2.0f, 10000.0f);
  /* dans quelle partie de l'écran on effectue le rendu */
  glViewport(0, 0, _wW, _wH);


  //-----------------------------------
  for(int i = 0; i< TENSOR_SIZE; i++)
    {
        for(int j = 0; j< TENSOR_SIZE; j++)
        {
            for(int k = 0; k< TENSOR_DEPTH; k++)
            {
                tensor[i][j][k].selfTime += PI / 4.0f;
                tensor[i][j][k].z = 0;
                tensor[i][j][k].go = 0;
            }
        }
    }
}

void keyd(int keycode) {
  switch(keycode) {
  case GL4DK_w:
    _wireframe = !_wireframe;
    glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
    break;

  case GL4DK_UP:
    _eyeZ += _move;
    _centerZ += _move;
    break;
  case GL4DK_DOWN:
    _eyeZ -= _move;
    _centerZ -= _move;
    break;
  case GL4DK_LEFT:
    _eyeX -= _move;
    _centerX -= _move;
    break;
  case GL4DK_RIGHT:
    _eyeX += _move;
    _centerX += _move;
    break;
  case GL4DK_u:
    _eyeY += _move;
    _centerY += _move;
    break;
  case GL4DK_d:
    _eyeY -= _move;
    _centerY -= _move;
    break;

  case GL4DK_l:
    _centerY -= _move;
    break;

  case GL4DK_m:
    _centerY += _move;
    break;

  case GL4DK_o:
    _centerX -= _move;
    break;

  case GL4DK_p:
    _centerX += _move;
    break;

  case GL4DK_b:
    _blur += 1;
    break;

  case GL4DK_n:
    _blur -= 1;
    if(_blur < 0)
      _blur = 1;
    break;

  case GL4DK_t:
    glBindTexture(GL_TEXTURE_2D, _texId);
    tex[0] = B;
    tex[1] = N;
    tex[2] = N;
    tex[3] = B;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    break;
  
  case GL4DK_y:
    glBindTexture(GL_TEXTURE_2D, _texId);
    tex[0] = B;
    tex[1] = B;
    tex[2] = B;
    tex[3] = B;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    break;

  case GL4DK_a:
    

    convolution();
    glBindTexture(GL_TEXTURE_2D, _texId);
    writeTex();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TENSOR_SIZE, TENSOR_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    break;  

  case GL4DK_q:
    addPerturbation(50,30,10);
    addPerturbation(20,10,20);
    addPerturbation(60,50,5);
    addPerturbation(70,20,15);

    // addPerturbation(2,2,1);
    break;

  default:
    break;
  }
}

/*!\brief Cette fonction dessine dans le contexte OpenGL actif. */
static void draw(void) {
  static float angle = 0.0f;
  static double t0 = 0;
  double t = gl4dGetElapsedTime(), dt = (t - t0) / 1000.0;
  t0 = t;
  /* effacement du buffer de couleur, nous rajoutons aussi le buffer
   * de profondeur afin de bien rendre les fragments qui sont devant
   * au dessus de ceux qui sont derrière. Commenter le
   * "|GL_DEPTH_BUFFER_BIT" pour voir la différence. Nous avons ajouté
   * l'activation du test de profondeur dans la fonction init via
   * l'appel glEnable(GL_DEPTH_TEST). */


  convolution();
  glBindTexture(GL_TEXTURE_2D, _texId);
  writeTex();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TENSOR_SIZE, TENSOR_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);


  
  // glClear(GL_COLOR_BUFFER_BIT);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /* activation du programme _pId */
  glUseProgram(_pId);
  /* lier (mettre en avant ou "courante") la matrice vue créée dans
   * init */
  gl4duBindMatrix("viewMatrix");
  /* Charger la matrice identité dans la matrice courante (liée) */
  gl4duLoadIdentityf();
  /* Composer la matrice vue courante en simulant une "caméra" à
   * l'aide de la fonction LookAt(xyz_position_cam,
   * xyz_ou_elle_regarde, xyz_son_vecteur_haut) */
  gl4duLookAtf(_eyeX, _eyeY, _eyeZ, _centerX, _centerY, _centerZ, 0.0f, 1.0f, 0.0f);
  /* lier (mettre en avant ou "courante") la matrice modèle créée dans
   * init */
  gl4duBindMatrix("modelMatrix");
  /* Charger la matrice identité dans la matrice courante (liée) */
  gl4duLoadIdentityf();

  /* Envoyer, au shader courant, toutes les matrices connues dans
   * GL4Dummies, ici on intègre pas la rotation qui vient après */
  gl4duSendMatrices();

  glUniform2f(glGetUniformLocation(_pId, "offset"), 1.0f / (_tw - 1.0f), 1.0f / (_th - 1.0f));
  glUniform1f(glGetUniformLocation(_pId, "temps"), t / 1000.0);

  glUniform3f(glGetUniformLocation(_pId, "alti"), 1.0f,1.0f,1.0f);


  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _texId);
  glUniform1i(glGetUniformLocation(_pId, "tex"), 0);
  
  /* dessiner la géométrie */

  gl4dgDraw(_gridId);
  // gl4dgDraw(_grid2Id);

  gl4dfBlur (0, 0, _blur, 1, 0, GL_FALSE);

  glBindTexture(GL_TEXTURE_2D, 0);

  /* désactiver le programme shader */
  glUseProgram(0);

  /* un tour par seconde */
  angle += 72.0f * dt;
}

/*!\brief appelée au moment de sortir du programme (atexit), elle
 *  libère les éléments OpenGL utilisés.*/
static void quit(void) {
  if(_texId) {
    glDeleteTextures(1, &_texId);
    _texId = 0;
  }
  /* nettoyage des éléments utilisés par la bibliothèque GL4Dummies */
  gl4duClean(GL4DU_ALL);
}



static void addPerturbation(int x, int y, float vel_z)
{
  for(int i = 0; i< TENSOR_SIZE; i++)
  {
    for(int j = 0; j< TENSOR_SIZE; j++)
    {
      tensor[i][j][nbWave].go = 0;
      tensor[i][j][nbWave].selfTime = 0;
      tensor[i][j][nbWave].z = 0;
    }
  }
  tensor[x][y][nbWave].z = vel_z;
  tensor[x][y][nbWave].go = 1;
  nbWave = (nbWave + 1)% TENSOR_DEPTH;
}

static void convolution()
{
    for(int i = 0; i< TENSOR_SIZE; i++)
    {
        for(int j = 0; j< TENSOR_SIZE; j++)
        {
            for(int k = 0; k< TENSOR_DEPTH; k++)
            {
                tmp_tensor[i][j][k].z = tensor[i][j][k].z;
                tmp_tensor[i][j][k].go = tensor[i][j][k].go;
                tmp_tensor[i][j][k].selfTime = tensor[i][j][k].selfTime;

                //CONVOLUTION
                float accZ = 0.0f;
                for (int _i = -1; _i < 2; _i++)
                {
                    for (int _j = -1; _j < 2; _j++)
                    {
                        if(i + _i >= 0 && i + _i <TENSOR_SIZE &&
                            j + _j >= 0 && j + _j <TENSOR_SIZE)
                        {
                            accZ += tensor[i+_i][j+_j][k].z * kernel[_i+1][_j+1];
                        }
                    }
                }

                if(tensor[i][j][k].go == 0 && accZ > 0.0f)
                {
                    tensor[i][j][k].go = 1;
                    tmp_tensor[i][j][k].z = accZ;
                }
            }
        }
    }

    for(int i = 0; i< TENSOR_SIZE; i++)
        for(int j = 0; j< TENSOR_SIZE; j++)
            for(int k = 0; k< TENSOR_DEPTH; k++)
                tensor[i][j][k].z = tmp_tensor[i][j][k].z;
}

int clamp(int d, int min, int max) {
  int t = d < min ? min : d;
  return t > max ? max : t;
}

static void writeTex()
{
  int count = 0;
  for(int i = 0; i< TENSOR_SIZE; i++)
  {
      for(int j= 0; j< TENSOR_SIZE; j++)
      {
          float accZ = 0;
          for(int k = 0; k< TENSOR_DEPTH; k++)
          {   
              
            if(tensor[i][j][k].selfTime < 2 * PI && tensor[i][j][k].go)
            {
              accZ +=  sinf(tensor[i][j][k].selfTime) * tensor[i][j][k].z;
              // tensor[i][j][k].selfTime += PI / 4.0f;
              tensor[i][j][k].selfTime += 0.1f;
            }

          }


          int height = clamp(accZ*255 + 125, 0, 255);
          tex[count] = RGB(height,0,0);

          // if(accZ > 0)
          //     tex[count] = B;
          // else if(accZ < 0)
          //     tex[count] = N;
          // else
          //     tex[count] = G;
          
          

          count++;
      }
  }
}