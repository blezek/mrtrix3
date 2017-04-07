/*

CFLAGS="-I/Users/mra9161/Downloads/ospray-1.2.1.x86_64.macosx/include" \
LDFLAGS="-Wl,-rpath,/Users/mra9161/Downloads/ospray-1.2.1.x86_64.macosx/lib -L/Users/mra9161/Downloads/ospray-1.2.1.x86_64.macosx/lib -lospray -lospray_commandline -lospray_common -lospray_importer -lospray_xml" \
./configure

release/bin/mrray ~/Data/3d-printing/segmented/3d-printing-017/tractography/small-probabalistic-tractography.tck

*/
  
#include <iostream>

#include "mrtrix.h"
#include "file/ofstream.h"
#include "file/name_parser.h"
#include "dwi/tractography/file.h"
#include "dwi/tractography/properties.h"

#include "app.h"
#include "cmdline_option.h"

#include <ospray/ospray.h>
#include <ospray/ospcommon/vec.h>

using namespace MR;
using namespace App;
using namespace MR::DWI::Tractography;
using namespace ospcommon;


// helper function to write the rendered image as PPM file
void writePPM(const char *fileName,
              const osp::vec2i &size,
              const uint32_t *pixel)
{
  FILE *file = fopen(fileName, "wb");
  fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
  unsigned char *out = (unsigned char *)alloca(3*size.x);
  for (int y = 0; y < size.y; y++) {
    const unsigned char *in = (const unsigned char *)&pixel[(size.y-1-y)*size.x];
    for (int x = 0; x < size.x; x++) {
      out[3*x + 0] = in[4*x + 0];
      out[3*x + 1] = in[4*x + 1];
      out[3*x + 2] = in[4*x + 2];
    }
    fwrite(out, 3*size.x, sizeof(char), file);
  }
  fprintf(file, "\n");
  fclose(file);
}

void usage() {
  MR::App::AUTHOR = "Daniel Blezek (daniel.blezek@gmail.com)";
  MR::App::OPTIONS
    // + MR::App::Argument ( "tracks", "Render the tracks" )
    // + MR::App::Argument ( "track" ).type_tracks_in().allow_multiple()
    
    + MR::App::Option ( "eye", "eye position, specified as a comma-separated list")
    + MR::App::Argument ("location").type_sequence_float()
    
    + MR::App::Option ( "gaze", "eye gaze location, specified as a comma-separated list")
    + MR::App::Argument ("location").type_sequence_float()
    
    + MR::App::Option ( "up", "eye up direction, specified as a comma-separated list")
    + MR::App::Argument ("direction").type_sequence_float()
    
    + MR::App::Option ( "width", "output width")
    + MR::App::Argument("w").type_integer(1024)
    
    + MR::App::Option ( "height", "output width")
    + MR::App::Argument("h").type_integer(1024)
    
    + MR::App::Option ( "output", "output image")
    + MR::App::Argument("output").type_file_out()
    ;
  
  MR::App::ARGUMENTS
    + MR::App::Argument ("models", "models").type_file_in().allow_multiple().optional();
}

int main( int argc, const char** argv ) {

  try {
    // initialize OSPRay; OSPRay parses (and removes) its commandline parameters, e.g. "--osp:debug"
    ospInit(&argc, argv);
    
    // Init MRtrix
    ::MR::App::init(argc,argv);
    usage();
    ::MR::App::parse();

  // camera
  float cam_pos[] = {0.f, 0.f, 0.f};
  float cam_up [] = {0.f, 1.f, 0.f};
  float cam_view [] = {0.f, 0.f, 1.f};
  osp::vec2i imgSize;
  imgSize.x = 1024; // width
  imgSize.y = 768; // height

  
  OSPCamera camera = ospNewCamera("perspective");
  ospSetf(camera, "aspect", imgSize.x/(float)imgSize.y);
  ospSet3fv(camera, "pos", cam_pos);
  ospSet3fv(camera, "dir", cam_view);
  ospSet3fv(camera, "up",  cam_up);
  ospCommit(camera); // commit each object to indicate modifications are done

  // create renderer
  OSPRenderer renderer = ospNewRenderer("scivis"); // choose Scientific Visualization renderer
  auto material = ospNewMaterial(renderer, "OBJMaterial");
  
  OSPModel world = ospNewModel();
  /*
  // Load the streamlines
  for ( auto arg : MR::App::argument ) {
    std::cout << "Loading " << arg << "\n";
    Properties properties;
    auto reader = new Reader<float>(argument[0], properties);
    std::vector<vec3fa> v;
    std::vector<vec4f> color;
    std::vector<int32_t> index;
    Streamline<float> tck;
    int32_t current_index = 0;
    while ( (*reader)(tck) ) {
      for ( size_t i = 0; i < tck.size() - 2; i++ ) {
        auto pos = tck[i];
        auto posNext = tck[i+1];
        v.push_back ( vec3fa(pos[0],pos[1],pos[2]));
        auto c = vec3fa(pos[0],pos[1],pos[2]) - vec3fa(posNext[0],posNext[1],posNext[2]);
        c = normalize(c);
        c = abs(c);
        color.push_back ( vec4f(c[0],c[1],c[2],1.0) );
        index.push_back(current_index);
        current_index++;
      }
      // Last vertex
      current_index++;
    }
    // Create the OSP structure
    auto streamline = ospNewGeometry("streamlines");
    // see http://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array-in-c
    auto vertexData = ospNewData(v.size(), OSP_FLOAT3, &v[0]);
    ospCommit(vertexData);
    auto indexData = ospNewData(index.size(), OSP_INT, &index[0]);
    ospCommit(indexData);
    auto colorData = ospNewData(color.size(), OSP_FLOAT3, &color[0]);
    ospCommit(colorData);
    ospSetData(streamline,"vertex",vertexData);
    ospSetData(streamline,"index",indexData);
    ospSetData(streamline,"vertex.color",colorData);
    ospAddGeometry(world, streamline);
    ospSetMaterial(streamline, material);
    ospCommit(streamline);
    std::cout << "\tAdded " << v.size() << " vertices" << "\n";
  }
  */

  // triangle mesh data
  float vertex[] = { -1.0f, -1.0f, 3.0f, 0.f,
                     -1.0f,  1.0f, 3.0f, 0.f,
                      1.0f, -1.0f, 3.0f, 0.f,
                      0.1f,  0.1f, 0.3f, 0.f };
  float color[] =  { 0.9f, 0.5f, 0.5f, 1.0f,
                     0.8f, 0.8f, 0.8f, 1.0f,
                     0.8f, 0.8f, 0.8f, 1.0f,
                     0.5f, 0.9f, 0.5f, 1.0f };
  int32_t index[] = { 0, 1, 2,
                      1, 2, 3 };
  // create and setup model and mesh
  OSPGeometry mesh = ospNewGeometry("triangles");
  OSPData data = ospNewData(4, OSP_FLOAT3A, vertex); // OSP_FLOAT3 format is also supported for vertex positions
  ospCommit(data);
  ospSetData(mesh, "vertex", data);

  data = ospNewData(4, OSP_FLOAT4, color);
  ospCommit(data);
  ospSetData(mesh, "vertex.color", data);

  data = ospNewData(2, OSP_INT3, index); // OSP_INT4 format is also supported for triangle indices
  ospCommit(data);
  ospSetData(mesh, "index", data);

  ospCommit(mesh);
  // ospAddGeometry(world, mesh);

  auto spheres = ospNewGeometry("spheres");
  ospSet1f(spheres,"radius", 0.3);
  
  float sphereCenters[] = { -1.0f, -1.0f, 3.0f, 0.f,
                     -1.0f,  1.0f, 3.0f, 0.f,
                      1.0f, -1.0f, 3.0f, 0.f,
                      0.1f,  0.1f, 0.3f, 0.f };
  auto sphereData = ospNewData(4, OSP_FLOAT3A, sphereCenters);
  ospCommit(sphereData);
  ospSetData(spheres,"spheres",sphereData);
  ospSetMaterial(spheres,material);
  ospCommit(spheres);
  
  ospAddGeometry(world,spheres);

  std::cout << "Created spheres\n";

  
  
  ospCommit(world);

  // create and setup light for Ambient Occlusion
  OSPLight light = ospNewLight(renderer, "ambient");
  ospCommit(light);
  OSPData lights = ospNewData(1, OSP_LIGHT, &light);
  ospCommit(lights);

  // complete setup of renderer
  ospSet1i(renderer, "aoSamples", 1);
  ospSet3f(renderer, "bgColor", 0.0, 0.0, 0.2);
  ospSetObject(renderer, "model",  world);
  ospSetObject(renderer, "camera", camera);
  ospSetObject(renderer, "lights", lights);
  ospCommit(renderer);
  
  OSPFrameBuffer framebuffer = ospNewFrameBuffer(imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
  ospFrameBufferClear(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);

  // render one frame
  ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

  // access framebuffer and write its content as PPM file
  const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
  writePPM("firstFrame.ppm", imgSize, fb);
  ospUnmapFrameBuffer(fb, framebuffer);
    
  }  
  catch (MR::Exception& E) { 
    E.display(); 
    return 1; 
  } 
  return 0;
}


