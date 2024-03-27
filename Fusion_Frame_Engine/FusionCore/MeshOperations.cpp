#include "MeshOperations.h"
#include "fstream"
#include <chrono>

FUSIONCORE::HalfEdge* CreateNewEdge(FUSIONCORE::Vertex* vertex1 , FUSIONCORE::Vertex* vertex2 , 
	                                std::vector<std::shared_ptr<FUSIONCORE::HalfEdge>> &HalfEdges , 
	                                std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, FUSIONCORE::PairVec3Hash> &EdgeMap)
{
	HalfEdges.emplace_back(std::make_shared<FUSIONCORE::HalfEdge>());
	EdgeMap[{vertex1->Position, vertex2->Position}] = HalfEdges.size() - 1;
	FUSIONCORE::HalfEdge* looseEdgePtr = HalfEdges[HalfEdges.size() - 1].get();
	looseEdgePtr->StartingVertex = vertex1;
	looseEdgePtr->EndingVertex = vertex2;
	vertex1->halfEdge = looseEdgePtr;
	return looseEdgePtr;
}

bool FUSIONCORE::MESHOPERATIONS::ExportObj(const char* FilePath, FUSIONCORE::Model& model)
{
	try
	{
		std::ofstream obj_file(FilePath);

		if (!obj_file.is_open()) {
			LOG_ERR("Error: Couldn't open file for writing.");
			return false;
		}

		obj_file << "#FusionFrame Engine" << std::endl;
		obj_file << std::endl;

		int MeshCount = model.Meshes.size();
		int IndexOffset = 1;
		for (size_t i = 0; i < MeshCount; i++)
		{
			auto& mesh = model.Meshes[i];
			auto& Vertices = mesh.GetVertices();
			auto& Faces = mesh.GetFaces();

			if (!mesh.MeshName.empty())
			{
				obj_file << "o " << mesh.MeshName << std::endl;
			}
			else
			{
				obj_file << "o mesh_" << i << std::endl;
			}
			for (auto& vertex : Vertices)
			{
				obj_file << "v " << vertex->Position.x << " " << vertex->Position.y << " " << vertex->Position.z << std::endl;
			}
			for (auto& vertex : Vertices)
			{
				obj_file << "vn " << vertex->Normal.x << " " << vertex->Normal.y << " " << vertex->Normal.z << std::endl;
			}
			for (auto& vertex : Vertices)
			{
				obj_file << "vt " << vertex->TexCoords.x << " " << vertex->TexCoords.y << std::endl;
			}
			for (auto& face : Faces)
			{
				auto& indices = face->Indices;
				obj_file << "f ";
				for (size_t j = 0; j < indices.size(); j++)
				{
					obj_file << indices[j] + IndexOffset << "/" << indices[j] + IndexOffset << "/" << indices[j] + IndexOffset << " ";
				}
				obj_file << std::endl;
			}
			IndexOffset += mesh.GetVertices().size();
		}

		obj_file.close();
	}
	catch (const std::exception& e)
	{
		LOG_ERR("Exception while exporting obj file[" << FilePath << "] :: " << e.what());
		return false;
	}
	return true;
}

void FUSIONCORE::MESHOPERATIONS::CalculateTangentBitangent(std::vector<std::shared_ptr<FUSIONCORE::Vertex>>& vertices, std::vector<unsigned int>& indices) 
{
	for (size_t i = 0; i < indices.size(); i += 3) {
		unsigned int i0 = indices[i];
		unsigned int i1 = indices[i + 1];
		unsigned int i2 = indices[i + 2];

		glm::vec3 edge1 = vertices[i1]->Position - vertices[i0]->Position;
		glm::vec3 edge2 = vertices[i2]->Position - vertices[i0]->Position;

		glm::vec2 deltaUV1 = vertices[i1]->TexCoords - vertices[i0]->TexCoords;
		glm::vec2 deltaUV2 = vertices[i2]->TexCoords - vertices[i0]->TexCoords;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		glm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		glm::vec3 bitangent;
		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		vertices[i0]->Tangent += tangent;
		vertices[i1]->Tangent += tangent;
		vertices[i2]->Tangent += tangent;

		vertices[i0]->Bitangent += bitangent;
		vertices[i1]->Bitangent += bitangent;
		vertices[i2]->Bitangent += bitangent;
	}

	for (size_t i = 0; i < vertices.size(); ++i) 
	{
		vertices[i]->Tangent = glm::normalize(vertices[i]->Tangent);
		vertices[i]->Bitangent = glm::normalize(vertices[i]->Bitangent);
	}
}

bool FUSIONCORE::MESHOPERATIONS::ImportObj(const char* FilePath, FUSIONCORE::Model& model)
{
	if (strlen(FilePath) == 0)
	{
		LOG_ERR("File path is empty!");
		return false;
	}
	std::string fileFormat = strrchr(FilePath, '.');
	LOG("fileFormat: " << fileFormat);
	if (fileFormat != std::string(".obj"))
	{
		LOG_ERR("Unsupported file format(" << fileFormat << ")!");
		return false;
	}
	try
	{
		std::ifstream inputFile(FilePath);
		if (!inputFile) {
			LOG_ERR("Error: Couldn't open file for reading.");
			return false;
		}
		std::string obj_file;
		std::string line;

		std::vector<std::vector<glm::vec3>> Positions;
		std::vector<std::vector<glm::vec3>> Normals;
		std::vector<std::vector<glm::vec2>> TextureCoordinates;

		std::vector<std::vector<std::shared_ptr<FUSIONCORE::Vertex>>> MeshVertecies;
		std::vector<std::vector<unsigned int>> MeshIndicies;
		std::vector<std::vector<std::shared_ptr<FUSIONCORE::Face>>> MeshFaces;

		int CurrentMesh = -1;
		int IndexOffset = 1;
		while (std::getline(inputFile, line)) {
			if (line.find("#") == std::string::npos)
			{
				if (line.find("mtllib") != std::string::npos)
				{

				}
				else if (line.find("usemtl") != std::string::npos)
				{

				}
				else if (line.find("o") != std::string::npos || line.find("g") != std::string::npos)
				{
					CurrentMesh++;
					Positions.emplace_back();
					Normals.emplace_back();
					TextureCoordinates.emplace_back();

					if (!MeshVertecies.empty())
					{
					  IndexOffset += MeshVertecies.back().size();
					}
					MeshVertecies.emplace_back();
					MeshIndicies.emplace_back();
					MeshFaces.emplace_back();
				}
				else if (line.find('s') != std::string::npos)
				{

				}
				else
				{
					if (line.find('vn') != std::string::npos)
					{
						//LOG(line);
						glm::vec3 Normal;
						std::vector<std::string> NormalsString;
						std::string TypeName("vn");
						size_t VtPos = line.find(TypeName);
						for (size_t i = VtPos + TypeName.size(); i < line.size(); i++)
						{
							if (line[i] == ' ')
							{
								if (i < line.size() - 1)
								{
									if (line[i + 1] != ' ')
									{
										NormalsString.emplace_back();
									}
								}
							}
							else
							{
								NormalsString.back() += line[i];
							}
						}
						for (size_t i = 0; i < 3; i++)
						{
							Normal[i] = std::stof(NormalsString[i]);
						}
						//LOG("Normal: " << Vec3<float>(Normal));
						Normals[CurrentMesh].push_back(glm::normalize(Normal));
					}
					else if (line.find('vt') != std::string::npos)
					{
						glm::vec2 TextureCoord;
						std::vector<std::string> TextureCoordsString;
						std::string TypeName("vt");
						size_t VtPos = line.find(TypeName);
						for (size_t i = VtPos + TypeName.size(); i < line.size(); i++)
						{
							if (line[i] == ' ')
							{
								if (i < line.size() - 1)
								{
									if (line[i + 1] != ' ')
									{
										TextureCoordsString.emplace_back();
									}
								}
							}
							else
							{
								TextureCoordsString.back() += line[i];
							}
						}
						for (size_t i = 0; i < 2; i++)
						{
							TextureCoord[i] = std::stof(TextureCoordsString[i]);
						}
						TextureCoordinates[CurrentMesh].push_back(TextureCoord);
					}
					else if (line.find('v') != std::string::npos)
					{
						glm::vec3 Position;
						std::vector<std::string> PositionsString;
						std::string TypeName("v");
						size_t VPos = line.find(TypeName);
						int CurrentElement = -1;
						for (size_t i = VPos + TypeName.size(); i < line.size(); i++)
						{
							if (line[i] == ' ')
							{
								if (i < line.size() - 1)
								{
									if (line[i + 1] != ' ')
									{
										PositionsString.emplace_back();
									}
								}
							}
							else
							{
								PositionsString.back() += line[i];
							}
						}

						for (size_t i = 0; i < 3; i++)
						{
							Position[i] = std::stof(PositionsString[i]);
						}

						Positions[CurrentMesh].push_back(Position);
					}
					else if (line.find('f') != std::string::npos)
					{
						size_t fPos = line.find("f");
						std::vector<std::string> Vertices;
						for (size_t i = fPos + 1; i < line.size(); i++)
						{
							if (line[i] == ' ' || line[i] == '/')
							{
								if (i < line.size() - 1)
								{
									if (line[i + 1] != ' ' && line[i + 1] != '/')
									{
									  Vertices.emplace_back();
									}
								}
							}
							else
							{
								Vertices.back() += line[i];
							}
						}

						FUSIONCORE::Face newFace;
						newFace.Indices.reserve(3);
						for (size_t i = 0; i < Vertices.size(); i += 3)
						{
							FUSIONCORE::Vertex newVertex;
							unsigned int index1 = std::stoi(Vertices[i]) - IndexOffset;
							unsigned int index2 = std::stoi(Vertices[i + 1]) - IndexOffset;
							unsigned int index3 = std::stoi(Vertices[i + 2]) - IndexOffset;

							newVertex.Position = Positions[CurrentMesh][index1];
							newVertex.TexCoords = TextureCoordinates[CurrentMesh][index2];
							newVertex.Normal = Normals[CurrentMesh][index3];
							MeshVertecies[CurrentMesh].push_back(std::make_shared<FUSIONCORE::Vertex>(newVertex));

							int CurrentVertexIndex = MeshIndicies[CurrentMesh].size();

							newFace.Indices.push_back(CurrentVertexIndex);
							MeshIndicies[CurrentMesh].push_back(CurrentVertexIndex);
						}
						MeshFaces[CurrentMesh].push_back(std::make_shared<FUSIONCORE::Face>(newFace));
					}
				}
			}
		}
		inputFile.close();

		std::vector<FUSIONCORE::Texture2D> textures;
		for (size_t i = 0; i < MeshVertecies.size(); i++)
		{
			model.Meshes.emplace_back(MeshVertecies[i], MeshIndicies[i], MeshFaces[i], textures);
			CalculateTangentBitangent(model.Meshes.back().GetVertices(), model.Meshes.back().GetIndices());
		}
		model.FindGlobalMeshScales();
	}
	catch (const std::exception& e)
	{
		LOG_ERR("Exception while reading obj file[" << FilePath << "] :: " << e.what());
		return false;
	}
	return true;
}

void FUSIONCORE::MESHOPERATIONS::SmoothObject(FUSIONCORE::Mesh& mesh)
{
	auto& Vertices = mesh.GetVertices();
	auto& DuplicateVertexMap = mesh.GetDuplicateVertexMap();
	std::vector<std::shared_ptr<FUSIONCORE::Vertex>> SmoothedVertices;
	SmoothedVertices.reserve(Vertices.size());
	for (size_t i = 0; i < Vertices.size(); i++)
	{
		auto& vertex = Vertices[i];

		glm::vec3 AveragedPosition;
		int EdgeIterationCount = 0;
		auto RelatedEdge = vertex->halfEdge;
		do
		{
			AveragedPosition += RelatedEdge->EndingVertex->Position;
			AveragedPosition += RelatedEdge->StartingVertex->Position;
			EdgeIterationCount += 2;
			if (!RelatedEdge->BoundryEdge)
			{
			    AveragedPosition += RelatedEdge->EndingVertex->Position;
				AveragedPosition += RelatedEdge->StartingVertex->Position;
				EdgeIterationCount += 2;
				RelatedEdge = RelatedEdge->TwinHalfEdge->NextHalfEdge;
			}
			else
			{
				LOG("STILL IN LOOP");
				if (RelatedEdge->PrevHalfEdge->BoundryEdge)
				{
					LOG("SHOULD BREAK");

					break;
				}
				RelatedEdge = RelatedEdge->PrevHalfEdge;
				AveragedPosition += RelatedEdge->EndingVertex->Position;
				AveragedPosition += RelatedEdge->StartingVertex->Position;
				EdgeIterationCount += 2;
				RelatedEdge = RelatedEdge->TwinHalfEdge;
			}
		} while (RelatedEdge != vertex->halfEdge);

		AveragedPosition /= EdgeIterationCount;
		/*if (DuplicateVertexMap.find(vertex->Position) != DuplicateVertexMap.end())
		{
			auto& DuplicateVertices = DuplicateVertexMap[vertex->Position];
			for (size_t y = 0; y < DuplicateVertices.size(); y++)
			{
				DuplicateVertices[y]->Position = AveragedPosition;
			}
		}*/
		vertex->Position = AveragedPosition;
		SmoothedVertices.push_back(vertex);
		LOG("EdgeIterationCount: " << EdgeIterationCount);
	}
	std::swap(Vertices, SmoothedVertices);
}


void FUSIONCORE::MESHOPERATIONS::LoopSubdivision(FUSIONCORE::Mesh& Mesh, int level)
{
	if (level == 0)
	{
		return;
	}
	auto& Faces = Mesh.GetFaces();
	auto& EdgeMap = Mesh.GetEdgeHashMap();
	auto& HalfEdges = Mesh.GetHalfEdges();
	auto& Vertices = Mesh.GetVertices();
	auto& Indicies = Mesh.GetIndices();

	std::unordered_map<glm::vec3, unsigned int, FUSIONCORE::Vec3Hash> VertexIndexMap;
	std::vector<unsigned int> ResultantIndices;

	size_t FacesCount = Faces.size();
	for (size_t FaceIndex = 0; FaceIndex < FacesCount; FaceIndex++)
	{
		FUSIONCORE::Face* face = Faces[FaceIndex].get();

		std::vector<FUSIONCORE::HalfEdge*> InputEdges;
		InputEdges.resize(3);
		InputEdges[0] = face->halfEdge;
		InputEdges[1] = face->halfEdge->NextHalfEdge;
		InputEdges[2] = face->halfEdge->NextHalfEdge->NextHalfEdge;

		std::vector<FUSIONCORE::Vertex*> MidPointPtrs;
		MidPointPtrs.resize(3);

		std::vector<unsigned int> NewVertexIndices;
		NewVertexIndices.reserve(3);

		unsigned int InputVertexIndex1 = face->Indices[0];
		unsigned int InputVertexIndex2 = face->Indices[1];
		unsigned int InputVertexIndex3 = face->Indices[2];

		std::vector<std::shared_ptr<FUSIONCORE::HalfEdge>> ConnectedSubEdgePtrs;
		ConnectedSubEdgePtrs.resize(3);
		for (size_t i = 0; i < 3; i++)
		{
			auto& edge = InputEdges[i];
			auto midpoint = FUSIONCORE::GetAveragedVertex(*edge->StartingVertex, *edge->EndingVertex);

			FUSIONCORE::Vertex* midVertexptr;
			if (EdgeMap.find({ midpoint.Position ,edge->StartingVertex->Position}) == EdgeMap.end() &&
				EdgeMap.find({ edge->EndingVertex->Position , midpoint.Position }) == EdgeMap.end())
			{
				Vertices.push_back(std::make_shared<FUSIONCORE::Vertex>(midpoint));
				int VertexIndex = Vertices.size() - 1;
				NewVertexIndices.push_back(VertexIndex);
				VertexIndexMap[midpoint.Position] = VertexIndex;
				midVertexptr = Vertices[VertexIndex].get();
			}
			else
			{
				int Edgeindex = EdgeMap[{ midpoint.Position, edge->StartingVertex->Position}];
				midVertexptr = HalfEdges[Edgeindex]->StartingVertex;
				NewVertexIndices.push_back(VertexIndexMap[midVertexptr->Position]);
			}
			auto SubEdge1Pair = std::make_pair(edge->StartingVertex->Position, midpoint.Position);

			HalfEdges.emplace_back(std::make_shared<FUSIONCORE::HalfEdge>());
			EdgeMap[SubEdge1Pair] = HalfEdges.size() - 1;
			ConnectedSubEdgePtrs[i] = HalfEdges[EdgeMap[SubEdge1Pair]];

			ConnectedSubEdgePtrs[i]->StartingVertex = edge->StartingVertex;
			ConnectedSubEdgePtrs[i]->EndingVertex = midVertexptr;

			ConnectedSubEdgePtrs[i]->StartingVertex->halfEdge = ConnectedSubEdgePtrs[i].get();

			auto edgePair = std::make_pair(edge->StartingVertex->Position, edge->EndingVertex->Position);
			EdgeMap[{ midpoint.Position, edge->EndingVertex->Position }] = EdgeMap[edgePair];
			EdgeMap.erase(edgePair);

			edge->StartingVertex = midVertexptr;
			MidPointPtrs[i] = midVertexptr;

			edge->StartingVertex->halfEdge = edge;
		}

		FUSIONCORE::HalfEdge* looseEdgePtr1 = CreateNewEdge(MidPointPtrs[0], MidPointPtrs[1], HalfEdges, EdgeMap);
		FUSIONCORE::HalfEdge* looseEdgePtr2 = CreateNewEdge(MidPointPtrs[1], MidPointPtrs[2], HalfEdges, EdgeMap);
		FUSIONCORE::HalfEdge* looseEdgePtr3 = CreateNewEdge(MidPointPtrs[2], MidPointPtrs[0], HalfEdges, EdgeMap);

		FUSIONCORE::HalfEdge* looseEdgeTwinPtr1 = CreateNewEdge(MidPointPtrs[1], MidPointPtrs[0], HalfEdges, EdgeMap);
		FUSIONCORE::HalfEdge* looseEdgeTwinPtr2 = CreateNewEdge(MidPointPtrs[2], MidPointPtrs[1], HalfEdges, EdgeMap);
		FUSIONCORE::HalfEdge* looseEdgeTwinPtr3 = CreateNewEdge(MidPointPtrs[0], MidPointPtrs[2], HalfEdges, EdgeMap);

		/*MidPointPtrs[0]->halfEdge = looseEdgePtr1;
		MidPointPtrs[1]->halfEdge = looseEdgePtr2;
		MidPointPtrs[2]->halfEdge = looseEdgePtr3;*/

		std::vector<FUSIONCORE::HalfEdge*> BoundryEdges;
		BoundryEdges.reserve(6);

		BoundryEdges.push_back(ConnectedSubEdgePtrs[0].get());
		BoundryEdges.push_back(InputEdges[0]);
		BoundryEdges.push_back(ConnectedSubEdgePtrs[1].get());
		BoundryEdges.push_back(InputEdges[1]);
		BoundryEdges.push_back(ConnectedSubEdgePtrs[2].get());
		BoundryEdges.push_back(InputEdges[2]);

		Faces.emplace_back(std::make_shared<FUSIONCORE::Face>());
		FUSIONCORE::Face* newFace1 = Faces.back().get();

		Faces.emplace_back(std::make_shared<FUSIONCORE::Face>());
		FUSIONCORE::Face* newFace2 = Faces.back().get();

		Faces.emplace_back(std::make_shared<FUSIONCORE::Face>());
		FUSIONCORE::Face* newFace3 = Faces.back().get();

		//EDGE CONNECTING
		BoundryEdges[0]->NextHalfEdge = looseEdgeTwinPtr3;
		BoundryEdges[0]->PrevHalfEdge = BoundryEdges[5];
		
		looseEdgeTwinPtr3->NextHalfEdge = BoundryEdges[5];
		looseEdgeTwinPtr3->PrevHalfEdge = BoundryEdges[0];

		BoundryEdges[5]->NextHalfEdge = BoundryEdges[0];
		BoundryEdges[5]->PrevHalfEdge = looseEdgeTwinPtr3;


		looseEdgePtr1->NextHalfEdge = looseEdgePtr2;
		looseEdgePtr1->PrevHalfEdge = looseEdgePtr3;

		looseEdgePtr2->NextHalfEdge = looseEdgePtr3;
		looseEdgePtr2->PrevHalfEdge = looseEdgePtr1;

		looseEdgePtr3->NextHalfEdge = looseEdgePtr1;
		looseEdgePtr3->PrevHalfEdge = looseEdgePtr2;


		BoundryEdges[1]->NextHalfEdge = BoundryEdges[2];
		BoundryEdges[1]->PrevHalfEdge = looseEdgeTwinPtr1;

		BoundryEdges[2]->NextHalfEdge = looseEdgeTwinPtr1;
		BoundryEdges[2]->PrevHalfEdge = BoundryEdges[1];

		looseEdgeTwinPtr1->NextHalfEdge = BoundryEdges[1];
		looseEdgeTwinPtr1->PrevHalfEdge = BoundryEdges[2];


		looseEdgeTwinPtr2->NextHalfEdge = BoundryEdges[3];
		looseEdgeTwinPtr2->PrevHalfEdge = BoundryEdges[4];

		BoundryEdges[3]->NextHalfEdge = BoundryEdges[4];
		BoundryEdges[3]->PrevHalfEdge = looseEdgeTwinPtr2;

		BoundryEdges[4]->NextHalfEdge = looseEdgeTwinPtr2;
		BoundryEdges[4]->PrevHalfEdge = BoundryEdges[3];


		//FACE CONNECTING
		BoundryEdges[0]->Face = newFace1;
		looseEdgeTwinPtr3->Face = newFace1;
		BoundryEdges[5]->Face = newFace1;

		looseEdgePtr1->Face = newFace2;
		looseEdgePtr2->Face = newFace2;
		looseEdgePtr3->Face = newFace2;

		BoundryEdges[1]->Face = newFace3;
		looseEdgeTwinPtr1->Face = newFace3;
		BoundryEdges[2]->Face = newFace3;

		looseEdgeTwinPtr2->Face = face;
		BoundryEdges[3]->Face = face;
		BoundryEdges[4]->Face = face;

		newFace1->halfEdge = BoundryEdges[0];
		newFace2->halfEdge = looseEdgePtr1;
		newFace3->halfEdge = BoundryEdges[1];
		face->halfEdge = looseEdgeTwinPtr2;

		newFace1->Indices.push_back(InputVertexIndex1);
		newFace1->Indices.push_back(NewVertexIndices[0]);
		newFace1->Indices.push_back(NewVertexIndices[2]);

		newFace2->Indices.push_back(NewVertexIndices[0]);
		newFace2->Indices.push_back(NewVertexIndices[1]);
		newFace2->Indices.push_back(NewVertexIndices[2]);

		newFace3->Indices.push_back(NewVertexIndices[0]);
		newFace3->Indices.push_back(InputVertexIndex2);
		newFace3->Indices.push_back(NewVertexIndices[1]);

		face->Indices[0] = NewVertexIndices[2];
		face->Indices[1] = NewVertexIndices[1];
		face->Indices[2] = InputVertexIndex3;

		ResultantIndices.insert(ResultantIndices.end(), newFace1->Indices.begin(), newFace1->Indices.end());
		ResultantIndices.insert(ResultantIndices.end(), newFace2->Indices.begin(), newFace2->Indices.end());
		ResultantIndices.insert(ResultantIndices.end(), newFace3->Indices.begin(), newFace3->Indices.end());
		ResultantIndices.insert(ResultantIndices.end(), face->Indices.begin(), face->Indices.end());
    }

	int HowManyBoundry = 0;
	std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, FUSIONCORE::PairVec3Hash>::iterator itr;
	for (itr = EdgeMap.begin(); itr != EdgeMap.end(); itr++)
	{
		auto twinPair = std::make_pair(itr->first.second, itr->first.first);
		if (EdgeMap.find(twinPair) == EdgeMap.end())
		{
			HowManyBoundry++;
			HalfEdges[itr->second]->BoundryEdge = true;
		}
		else
		{
			HalfEdges[itr->second]->TwinHalfEdge = HalfEdges[EdgeMap[twinPair]].get();
			HalfEdges[itr->second]->BoundryEdge = false;
		}
	}
	LOG("BOUNDRY: " << HowManyBoundry);
	std::swap(ResultantIndices, Indicies);
	
	if (level > 1)
	{
		LoopSubdivision(Mesh, level - 1);
	}
	else
	{
		SmoothObject(Mesh);
		Mesh.ConstructMesh();
	}
}



void FUSIONCORE::MESHOPERATIONS::CollapseDecimation(FUSIONCORE::Mesh& Mesh, int level)
{
}

std::vector<glm::vec3> FUSIONCORE::MESHOPERATIONS::DistributePointsOnMeshSurface(FUSIONCORE::Mesh& Mesh, unsigned int PointCount, unsigned int seed)
{
	auto &MeshFaces = Mesh.GetFaces();
	auto& MeshVertices = Mesh.GetVertices();
	size_t CastedPointCount = 0;
	std::vector<glm::vec3> Points;

	Points.reserve(PointCount);
	while (CastedPointCount < PointCount)
	{
		for (size_t i = 0; i < MeshFaces.size(); i++)
		{
			auto& face = MeshFaces[i];
			auto& vertex1 = MeshVertices[face->Indices[0]];
			auto& vertex2 = MeshVertices[face->Indices[1]];
			auto& vertex3 = MeshVertices[face->Indices[2]];

			std::mt19937 gen(seed);
			std::uniform_real_distribution<double> dis(0.0, 1.0);

			double s = dis(gen);
			double t = dis(gen);

			glm::vec3 u = vertex1->Position;
			glm::vec3 v = vertex2->Position;
			glm::vec3 w = vertex3->Position;

			if (s + t <= 1) 
			{
				Points.push_back(glm::vec3(u.x + s * (v.x - u.x) + t * (w.x - u.x),
					                       u.y + s * (v.y - u.y) + t * (w.y - u.y),
					                       u.z + s * (v.z - u.z) + t * (w.z - u.z)));
			}
			else 
			{
				s = 1 - s;
				t = 1 - t;
				Points.push_back(glm::vec3(u.x + s * (w.x - u.x) + t * (v.x - u.x),
					                       u.y + s * (w.y - u.y) + t * (v.y - u.y),
					                       u.z + s * (w.z - u.z) + t * (v.z - u.z)));
			}

			CastedPointCount++;
			if (CastedPointCount >= PointCount)
			{
				break;
			}
		}
	}
	return Points;
}

std::vector<glm::vec3> FUSIONCORE::MESHOPERATIONS::DistributePointsOnMeshSurface(FUSIONCORE::Mesh& Mesh, FUSIONCORE::WorldTransform& Transformation, unsigned int PointCount, unsigned int seed)
{
	auto& MeshFaces = Mesh.GetFaces();
	auto& MeshVertices = Mesh.GetVertices();
	auto ModelMatrix = Transformation.GetModelMat4();
	size_t CastedPointCount = 0;
	std::vector<glm::vec3> Points;
	std::mt19937 gen(seed);

	Points.reserve(PointCount);
	while (CastedPointCount < PointCount)
	{
		for (size_t i = 0; i < MeshFaces.size(); i++)
		{
			auto& face = MeshFaces[i];
			auto& vertex1 = MeshVertices[face->Indices[0]];
			auto& vertex2 = MeshVertices[face->Indices[1]];
			auto& vertex3 = MeshVertices[face->Indices[2]];

			std::uniform_real_distribution<double> dis(0.0, 1.0);

			double s = dis(gen);
			double t = dis(gen);

			glm::vec3 u = vertex1->Position;
			glm::vec3 v = vertex2->Position;
			glm::vec3 w = vertex3->Position;

			if (s + t <= 1)
			{
				glm::vec3 Point = glm::vec3(u.x + s * (v.x - u.x) + t * (w.x - u.x),
					u.y + s * (v.y - u.y) + t * (w.y - u.y),
					u.z + s * (v.z - u.z) + t * (w.z - u.z));
				Points.push_back(FUSIONCORE::TranslateVertex(ModelMatrix, Point));
			}
			else
			{
				s = 1 - s;
				t = 1 - t;
				glm::vec3 Point = glm::vec3(u.x + s * (w.x - u.x) + t * (v.x - u.x),
					u.y + s * (w.y - u.y) + t * (v.y - u.y),
					u.z + s * (w.z - u.z) + t * (v.z - u.z));
				Points.push_back(FUSIONCORE::TranslateVertex(ModelMatrix, Point));
			}

			CastedPointCount++;
			if (CastedPointCount >= PointCount)
			{
				break;
			}
		}
	}
	return Points;
}

std::vector<FUSIONPHYSICS::CollisionBox> FUSIONCORE::MESHOPERATIONS::GridSubdivideCollisionBox(FUSIONPHYSICS::CollisionBox& collisionBox, unsigned int DivisionCountX, unsigned int DivisionCountY, unsigned int DivisionCountZ)
{
	/*unsigned int TotalNewBoxCount = DivisionCountX * DivisionCountY * DivisionCountZ;
	std::vector<FUSIONPHYSICS::CollisionBox> DividedBoxes;
	DividedBoxes.reserve(TotalNewBoxCount);

	auto& ParentTransformation = collisionBox.GetTransformation();
	auto ParentModelMatrix = ParentTransformation.GetModelMat4();



	return DividedBoxes;*/
}

void FUSIONCORE::MESHOPERATIONS::FillInstanceDataVBO(FUSIONCORE::VBO& DestVBO, std::vector<glm::vec3> &InstanceData)
{
	size_t InstanceCount = InstanceData.size();

	DestVBO.Bind();
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * InstanceCount, &InstanceData[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	DestVBO.SetVBOstate(true);
}
