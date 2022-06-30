#include <stdlib.h>
#include "pluginobj.h"

#include "stringutil/filenameutil.h"
#include "trimesh2/TriMesh.h"
#include "cxbin/convert.h"
#include "ccglobal/tracer.h"

namespace cxbin
{
	ObjLoader::ObjLoader()
	{

	}

	ObjLoader::~ObjLoader()
	{

	}

	std::string ObjLoader::expectExtension()
	{
		return "obj";
	}

	bool ObjLoader::tryLoad(FILE* f, size_t fileSize)
	{
		bool isObj = false;
		int times = 0;
		while (1) {
			stringutil::skip_comments(f);
			if (feof(f))
				break;
			char buf[1024];
			GET_LINE();
			if (LINE_IS("v ") || LINE_IS("v\t")) {
				float x, y, z;
				if (sscanf(buf + 1, "%f %f %f", &x, &y, &z) != 3) {
					break;
				}

				++times;
				if (times >= 3)
				{
					isObj = true;
					break;
				}
			}
		}

		return isObj;
	}

    void componentsSeparatedByString(std::string &from, char separator, std::vector<std::string> &out)
    {
        size_t leng = from.length();
        if (leng == 0) {
            return;
        }
        size_t location = 0;
        while (location < leng) {
            size_t tmp = from.find(separator, location);
            
            if (tmp == std::string::npos) {
                std::string sep = from.substr(location, leng-location);
                if (sep.length()) {
                    out.push_back(sep);
                }
                break;
            } else {
                std::string sep = from.substr(location, tmp-location);
                if (sep.length()) {
                    out.push_back(sep);
                }
                location = tmp + 1;
            }
        }
    }

    std::string trimStr(std::string& s)
    {
        size_t n = s.find_last_not_of(" \r\n\t");
        if (n != std::string::npos) {
            s.erase(n + 1, s.size() - n);
        }
        n = s.find_first_not_of(" \r\n\t");
        if (n != std::string::npos) {
            s.erase(0, n);
        }
        return s;
    }

	bool ObjLoader::load(FILE* f, size_t fileSize, std::vector<trimesh::TriMesh*>& out, ccglobal::Tracer* tracer)
    {
		unsigned count = 0;
		unsigned calltime = fileSize / 10;
		int num = 0;

         std::vector<trimesh::vec3> tmp_vertices;
         std::vector<trimesh::vec2> tmp_UVs;
         std::vector<trimesh::vec3> tmp_normals;
         auto remapVertexIndex = [tmp_vertices](int vi) { return (vi < 0) ? tmp_vertices.size() + vi : vi - 1; };
         auto remapNormalIndex = [tmp_normals](int vi) { return (vi < 0) ? tmp_normals.size() + vi : vi - 1; };
         auto remapTexCoordIndex=[tmp_UVs](int vi) { return (vi < 0) ? tmp_UVs.size() + vi : vi - 1; };

        std::vector<trimesh::Material> materials;
        std::vector<trimesh::ObjIndexedFace> indexedFaces;

        int  currentMaterialIdx = 0;
        trimesh::Material defaultMaterial;					
        defaultMaterial.index = currentMaterialIdx;
        materials.push_back(defaultMaterial);

		while (1) {
			if (tracer && tracer->interrupt())
				return false;

//			stringutil::skip_comments(f);
            
			if (feof(f))
				break;
            char buf[MAX_OBJ_READLINE_LEN] = { 0 };
//            GET_LINE();
            fgets(buf, MAX_OBJ_READLINE_LEN, f);

			std::string str = buf;
			count += str.length();
			if (tracer && count >= calltime)
			{
				count = 0;
				num++;
				tracer->progress((float)num / 10.0);
			}
			
			if (LINE_IS("v ") || LINE_IS("v\t")) {
				float x, y, z;
				if (sscanf(buf + 1, "%f %f %f", &x, &y, &z) != 3) {
					if (tracer)
						tracer->failed("sscanf failed");
					return false;
				}
                tmp_vertices.push_back(trimesh::point(x, y, z));
			}
			else if (LINE_IS("vn ") || LINE_IS("vn\t")) {
				float x, y, z;
				if (sscanf(buf + 2, "%f %f %f", &x, &y, &z) != 3) {
					if (tracer)
						tracer->failed("sscanf failed");
					return false;
				}
                tmp_normals.push_back(trimesh::vec(x, y, z));
			}
            else if (LINE_IS("vt") || LINE_IS("vt\t")) {
                float x, y;
                if (sscanf(buf + 2, "%f %f", &x, &y) != 2) {
                    return false;
                }
                tmp_UVs.push_back(trimesh::vec2(x, y));
            }
			else if (LINE_IS("f ") || LINE_IS("f\t")) {
				
                std::string strOfFace = str.substr(2, str.length()-2);
                const char* ptr = strOfFace.c_str();
                int vi = 0, ti = 0, ni = 0;
                trimesh::ObjIndexedFace   ff;
                while (*ptr != 0)
                {
                    // skip white space
                    while (*ptr == ' ') ++ptr;

                    if (sscanf(ptr, "%d/%d/%d", &vi, &ti, &ni) == 3)
                    {
                        ff.v.emplace_back(remapVertexIndex(vi));
                        ff.t.emplace_back(remapTexCoordIndex(ti));
                        ff.n.emplace_back(remapNormalIndex(ni));
                        ff.tInd = currentMaterialIdx;

                    }
                    else if (sscanf(ptr, "%d/%d", &vi, &ti) == 2)
                    {
                        ff.v.emplace_back(remapVertexIndex(vi));
                        ff.t.emplace_back(remapTexCoordIndex(ti));
                        ff.tInd = currentMaterialIdx;

                    }
                    else if (sscanf(ptr, "%d//%d", &vi, &ni) == 2)
                    {
                        ff.v.emplace_back(remapVertexIndex(vi));
                        ff.n.emplace_back(remapNormalIndex(ni));
                    }
                    else if (sscanf(ptr, "%d", &vi) == 1)
                    {
                        ff.v.emplace_back(remapVertexIndex(vi));
                    }

                    // skip to white space or end of line
                    while (*ptr != ' ' && *ptr != 0) ++ptr;

                }
                indexedFaces.push_back(ff);


			} 
            else if (LINE_IS("g ") || LINE_IS("o ")) {
                //结束上一个部件
//                if (model->faces.size() > 0) {
//                    model = new trimesh::TriMesh();
//                    out.push_back(model);
//                }
//
//                //开始下一个部件
//                std::string name = str.substr(2, str.length()-2);
//                name = trimStr(name);
                
            } 
            else if (LINE_IS("mtllib ")) {
                std::string name = str.substr(7, str.length()-7);
                name = trimStr(name);
                //mtl文件名
                
                size_t loc = modelPath.find_last_of("/");
                if (loc != std::string::npos) {
                    
                    const std::string materialFileName = modelPath.substr(0, loc) + "/" + name;
                    printf("[mtl]: %s\n", materialFileName.c_str());
                    
                    loadMtl(materialFileName, materials);
                }
                
                
            } 
            else if (LINE_IS("usemtl ")) {
                //mtllib should been readed before 
                std::string name = str.substr(7, str.length()-7);
                name = trimStr(name);
                
                if (name.empty()) {
                    continue;
                }

                // emergency check. If there are no materials, the material library failed to load or was not specified
                // but there are tools that save the material library with the same name of the file, but do not add the 
                // "mtllib" definition in the header. So, we can try to see if this is the case
                if ((materials.size() == 1) && (materials[0].name == "")) {
                    std::string materialFileName(modelPath);
                    materialFileName.replace(materialFileName.end() - 4, materialFileName.end(), ".mtl");
                    loadMtl(materialFileName, materials);
                }

                std::string materialName= name;

                bool found = false;
                unsigned i = 0;
                while (!found && (i < materials.size()))
                {
                    std::string currentMaterialName = materials[i].name;
                    if (currentMaterialName == materialName)
                    {
                        currentMaterialIdx = i;
                        trimesh::Material& material = materials[currentMaterialIdx];
                        trimesh::vec3 diffuseColor = material.diffuse;
                        unsigned char r = (unsigned char)(diffuseColor[0] * 255.0);
                        unsigned char g = (unsigned char)(diffuseColor[1] * 255.0);
                        unsigned char b = (unsigned char)(diffuseColor[2] * 255.0);
                        unsigned char alpha = (unsigned char)(material.Tr * 255.0);
                        trimesh::vec4 currentColor = trimesh::vec4(r, g, b, alpha);
                        found = true;
                    }
                    ++i;
                }

                if (!found)
                {
                    currentMaterialIdx = 0;
                }
            }

        }

		// XXX - FIXME
		// Right now, handling of normals is fragile: we assume that
		// if we have the same number of normals as vertices,
		// the file just uses per-vertex normals.  Otherwise, we can't
		// handle it.
        

        //if (tmp_vertices.size()*3== tmp_UVs.size()
        //    && tmp_UVs.size() == tmp_normals.size())
        {
            trimesh::TriMesh* modelmesh = new trimesh::TriMesh();
            std::swap(modelmesh->vertices, tmp_vertices);
            std::swap(modelmesh->UVs, tmp_UVs);
            for (int i = 0; i < indexedFaces.size(); i++)
            {
                trimesh::ObjIndexedFace& ff = indexedFaces[i];
                if (ff.v.size() == 3)
                {
                   modelmesh->faces.emplace_back(trimesh::TriMesh::Face(ff.v[0], ff.v[1], ff.v[2]));
                   if (ff.t.size() == 3)
                    {
                        modelmesh->faceUVs.emplace_back(trimesh::TriMesh::Face(ff.t[0], ff.t[1], ff.t[2]));
                        modelmesh->textureIDs.emplace_back(ff.tInd);
                    }
                    else
                    {
                        modelmesh->faceUVs.emplace_back(trimesh::TriMesh::Face(0, 0,0));
                        modelmesh->textureIDs.emplace_back(0);
                    }
                }
            }
            std::swap(modelmesh->m_materials, materials);
            
            out.push_back(modelmesh);
        }

		if (tracer)
		{
			tracer->success();
		}
		return true;
	}


    bool ObjLoader::loadMtl(const std::string& fileName, std::vector<trimesh::Material>& out)
    {
        FILE* f = fopen(fileName.c_str(), "rb");
        if (!f)
            return false;
        
        trimesh::Material *matePtr = nullptr;
        bool isValid = false;
        while (1) {
            stringutil::skip_comments(f);
            if (feof(f))
                break;
            char buf[1024];
            GET_LINE();
            std::string str = buf;
            if (LINE_IS("newmtl ")) {
                
                trimesh::Material newmat;
                newmat.index = out.size();
                out.push_back(newmat);
                
                matePtr = &out[out.size()-1];
                std::string name = str.substr(7, str.length()-7);
                matePtr->name = trimStr(name);
                
                isValid = true;
            } else if (LINE_IS("Ka ")) {
                
                if (!isValid) continue;
                
                float x, y, z;
                if (sscanf(buf + 2, "%f %f %f", &x, &y, &z) != 3) {
                    fclose(f);
                    return false;
                }
                matePtr->ambient = trimesh::vec3(x, y, z);
                
            } else if (LINE_IS("Kd ")) {
                
                if (!isValid) continue;
                
                float x, y, z;
                if (sscanf(buf + 2, "%f %f %f", &x, &y, &z) != 3) {
                    fclose(f);
                    return false;
                }
                matePtr->diffuse = trimesh::vec3(x, y, z);
                
            } else if (LINE_IS("Ks ")) {
                
                if (!isValid) continue;
                
                float x, y, z;
                if (sscanf(buf + 2, "%f %f %f", &x, &y, &z) != 3) {
                    fclose(f);
                    return false;
                }
                matePtr->specular = trimesh::vec3(x, y, z);
                
            } else if (LINE_IS("Ke ")) {
                
                if (!isValid) continue;
                
                float x, y, z;
                if (sscanf(buf + 2, "%f %f %f", &x, &y, &z) != 3) {
                    fclose(f);
                    return false;
                }
                matePtr->emission = trimesh::vec3(x, y, z);
                
            } else if (LINE_IS("Ns ")) {
                
                if (!isValid) continue;
                
                float x;
                if (sscanf(buf + 2, "%f", &x) != 1) {
                    fclose(f);
                    return false;
                }
                matePtr->shiness = x;
                
            } else if (LINE_IS("map_Ka ")) {
                
                if (!isValid) continue;
                
                std::string name = str.substr(7, str.length()-7);
                name = trimStr(name);
                std::vector<std::string> strs;
                componentsSeparatedByString(name, ' ', strs);
                if (strs.size()) {
                    size_t loc = fileName.find_last_of("/");
                    if (loc != std::string::npos) {

                        const std::string full = fileName.substr(0, loc) + "/" + strs[strs.size() - 1];
                        printf("[map_Ka]: %s\n", full.c_str());
                        matePtr->ambientMap = strs[strs.size() - 1];

                    }

                }
                
            } else if (LINE_IS("map_Kd ")) {
                
                if (!isValid) continue;
                
                std::string name = str.substr(7, str.length()-7);
                name = trimStr(name);
                std::vector<std::string> strs;
                componentsSeparatedByString(name, ' ', strs);
                if (strs.size()) {
                    size_t loc = fileName.find_last_of("/");
                    if (loc != std::string::npos) {

                        const std::string full = fileName.substr(0, loc) + "/" + strs[strs.size() - 1];
                        printf("[map_Kd]: %s\n", full.c_str());
                        matePtr->diffuseMap = full;

                    }

                }
                
            } else if (LINE_IS("map_Ks ")) {
                
                if (!isValid) continue;
                
                std::string name = str.substr(7, str.length()-7);
                name = trimStr(name);
                std::vector<std::string> strs;
                componentsSeparatedByString(name, ' ', strs);
                if (strs.size()) {
                    size_t loc = fileName.find_last_of("/");
                    if (loc != std::string::npos) {

                        const std::string full = fileName.substr(0, loc) + "/" + strs[strs.size() - 1];
                        printf("[map_Ks]: %s\n", full.c_str());
                        matePtr->specularMap = full;

                    }
                }
                
            } else if (LINE_IS("map_bump ")) {
            
                if (!isValid) continue;
                
                std::string name = str.substr(9, str.length()-9);
                name = trimStr(name);
                std::vector<std::string> strs;
                componentsSeparatedByString(name, ' ', strs);
                if (strs.size()) {
                    std::vector<std::string>::iterator it = strs.end()-1;
                    matePtr->normalMap = *it;
                }
                
            }
        }
        
        fclose(f);
            
        return true;
    }
}
