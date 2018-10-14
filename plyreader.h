#pragma once

class PlyReader {
public:
    auto readObj(const std::string& filename) {
        auto vertices = std::vector<float>();
        auto indices = std::vector<unsigned int>();

        auto inVertices = false;
        auto inFaces = false;

        auto file = std::ifstream(filename);

        std::string line;
        while (std::getline(file, line)) {
            if (line.find("element vertex") != std::string::npos)
            {
                auto lineStream = std::stringstream(line);
                int size;
                std::string nothing;
                lineStream >> nothing >> nothing >> size;

                vertices.reserve((size_t)size*3);
            }
            else if (line.find("element face") != std::string::npos)
            {
                auto lineStream = std::stringstream(line);
                int size;
                std::string nothing;
                lineStream >> nothing >> nothing >> size;

                indices.reserve((size_t)size*3);
            }
            else if (line.find("end_header") != std::string::npos)
            {
                inVertices = true;
            }
            else if (line.find("3 ") == 0)
            {
                inVertices = false;
                inFaces = true;

                auto lineStream = std::stringstream(line);
                unsigned int i1,i2,i3;
                lineStream >> i1 >> i1 >> i2 >> i3;

                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
            else if (inVertices)
            {
                auto lineStream = std::stringstream(line);
                float v1,v2,v3;
                lineStream >> v1 >> v2 >> v3;

                vertices.push_back(v1);
                vertices.push_back(v2);
                vertices.push_back(v3);
            }
            else if (inFaces)
            {
                auto lineStream = std::stringstream(line);
                unsigned int i1,i2,i3;
                lineStream >> i1 >> i1 >> i2 >> i3;

                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }

        auto normals = std::vector<float>();
        normals.reserve(vertices.size());
        for (auto i = 0ul; i < vertices.size(); i++)
            normals.push_back(0);

        auto crossProduct = [] (auto p1, auto p2, auto p3) {
            auto [p1x,p1y,p1z] = p1;
            auto [p2x,p2y,p2z] = p2;
            auto [p3x,p3y,p3z] = p3;

            auto v1x = p2x-p1x;
            auto v1y = p2y-p1y;
            auto v1z = p2z-p1z;
            auto v2x = p3x-p1x;
            auto v2y = p3y-p1y;
            auto v2z = p3z-p1z;

            float x, y, z;
            x = v1y * v2z - v2y * v1z;
            y = (v1x * v2z - v2x * v1z) * -1;
            z = v1x * v2y - v2x * v1y;

            return std::make_tuple(x,y,z);
        };
        for (auto i = 0ul; i < indices.size(); i+=3) {
            const auto& i1 = indices[i+0];
            const auto& i2 = indices[i+1];
            const auto& i3 = indices[i+2];

            auto p1 = std::make_tuple(
                    vertices[i1*3+0],
                    vertices[i1*3+1],
                    vertices[i1*3+2]);
            auto p2 = std::make_tuple(
                    vertices[i2*3+0],
                    vertices[i2*3+1],
                    vertices[i2*3+2]);
            auto p3 = std::make_tuple(
                    vertices[i3*3+0],
                    vertices[i3*3+1],
                    vertices[i3*3+2]);
            
            auto normal = crossProduct(p1,p2,p3);

            auto [x,y,z] = normal;

            normals[i1*3+0] += x;
            normals[i1*3+1] += y;
            normals[i1*3+2] += z;
            normals[i2*3+0] += x;
            normals[i2*3+1] += y;
            normals[i2*3+2] += z;
            normals[i3*3+0] += x;
            normals[i3*3+1] += y;
            normals[i3*3+2] += z;
        }

        auto normalize = [] (auto x, auto y, auto z) {
            auto len = sqrt(x*x + y*y + z*z);
            return std::make_tuple(x/len,y/len,z/len);
        };
        for (auto i = 0ul; i < normals.size(); i+=3) {
            auto x = normals[i+0]; 
            auto y = normals[i+1]; 
            auto z = normals[i+2]; 

            auto [xN,yN,zN] = normalize(x,y,z);

            normals[i+0] = xN;
            normals[i+1] = yN;
            normals[i+2] = zN;
        }

        //auto minx = std::numeric_limits<float>::max();
        //auto miny = std::numeric_limits<float>::max();
        //auto minz = std::numeric_limits<float>::max();
        //auto maxx = -std::numeric_limits<float>::min();
        //auto maxy = -std::numeric_limits<float>::min();
        //auto maxz = -std::numeric_limits<float>::min();
        //for (auto i = 0ul; i < vertices.size(); i+=3) {
        //    if (vertices[i+0] < minx) minx = vertices[i+0];
        //    if (vertices[i+1] < miny) miny = vertices[i+1];
        //    if (vertices[i+2] < minz) minz = vertices[i+2];
        //    if (vertices[i+0] > maxx) maxx = vertices[i+0];
        //    if (vertices[i+1] > maxy) maxy = vertices[i+1];
        //    if (vertices[i+2] > maxz) maxz = vertices[i+2];
        //}
        //std::cout << minx << " " << miny << " " << minz << std::endl;
        //std::cout << maxx << " " << maxy << " " << maxz << std::endl;

        return std::make_tuple(
                vertices, 
                indices, 
                normals);
    }
};
