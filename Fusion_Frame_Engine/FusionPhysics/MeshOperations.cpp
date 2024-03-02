#include "MeshOperations.h"

FUSIONCORE::HalfEdge* CreateNewEdge(FUSIONCORE::Vertex* vertex1 , FUSIONCORE::Vertex* vertex2 , 
	                                std::vector<std::shared_ptr<FUSIONCORE::HalfEdge>> &HalfEdges , 
	                                std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, FUSIONCORE::PairVec3Hash> &EdgeMap)
{
	HalfEdges.emplace_back(std::make_shared<FUSIONCORE::HalfEdge>());
	EdgeMap[{vertex1->Position, vertex2->Position}] = HalfEdges.size() - 1;
	FUSIONCORE::HalfEdge* looseEdgePtr2 = HalfEdges[HalfEdges.size() - 1].get();
	looseEdgePtr2->StartingVertex = vertex1;
	looseEdgePtr2->EndingVertex = vertex2;
	return looseEdgePtr2;
}

void FUSIONPHYSICS::MESHOPERATIONS::LoopSubdivision(FUSIONCORE::Mesh& Mesh, int level)
{
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

			auto edgePair = std::make_pair(edge->StartingVertex->Position, edge->EndingVertex->Position);
			EdgeMap[{ midpoint.Position, edge->EndingVertex->Position }] = EdgeMap[edgePair];
			EdgeMap.erase(edgePair);

			edge->StartingVertex = midVertexptr;
			MidPointPtrs[i] = midVertexptr;
		}

		FUSIONCORE::HalfEdge* looseEdgePtr1 = CreateNewEdge(MidPointPtrs[0], MidPointPtrs[2],HalfEdges,EdgeMap);
		FUSIONCORE::HalfEdge* looseEdgePtr2 = CreateNewEdge(MidPointPtrs[0], MidPointPtrs[1], HalfEdges, EdgeMap);
		FUSIONCORE::HalfEdge* looseEdgePtr3 = CreateNewEdge(MidPointPtrs[1], MidPointPtrs[2], HalfEdges, EdgeMap);

		FUSIONCORE::HalfEdge* looseEdgeTwinPtr1 = CreateNewEdge(MidPointPtrs[2], MidPointPtrs[0], HalfEdges, EdgeMap);
		FUSIONCORE::HalfEdge* looseEdgeTwinPtr2 = CreateNewEdge(MidPointPtrs[0], MidPointPtrs[1], HalfEdges, EdgeMap);
		FUSIONCORE::HalfEdge* looseEdgeTwinPtr3 = CreateNewEdge(MidPointPtrs[2], MidPointPtrs[1], HalfEdges, EdgeMap);

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
		BoundryEdges[0]->NextHalfEdge = looseEdgePtr1;
		BoundryEdges[0]->PrevHalfEdge = BoundryEdges[5];
		
		looseEdgePtr1->NextHalfEdge = BoundryEdges[5];
		looseEdgePtr1->PrevHalfEdge = BoundryEdges[0];

		BoundryEdges[5]->NextHalfEdge = BoundryEdges[0];
		BoundryEdges[5]->PrevHalfEdge = looseEdgePtr1;


		looseEdgeTwinPtr1->NextHalfEdge = looseEdgeTwinPtr2;
		looseEdgeTwinPtr1->PrevHalfEdge = looseEdgeTwinPtr3;

		looseEdgeTwinPtr2->NextHalfEdge = looseEdgeTwinPtr3;
		looseEdgeTwinPtr2->PrevHalfEdge = looseEdgeTwinPtr1;

		looseEdgeTwinPtr3->NextHalfEdge = looseEdgeTwinPtr1;
		looseEdgeTwinPtr3->PrevHalfEdge = looseEdgeTwinPtr2;


		BoundryEdges[1]->NextHalfEdge = BoundryEdges[2];
		BoundryEdges[1]->PrevHalfEdge = looseEdgePtr2;

		BoundryEdges[2]->NextHalfEdge = looseEdgePtr2;
		BoundryEdges[2]->PrevHalfEdge = BoundryEdges[1];

		looseEdgePtr2->NextHalfEdge = BoundryEdges[1];
		looseEdgePtr2->PrevHalfEdge = BoundryEdges[2];


		looseEdgePtr3->NextHalfEdge = BoundryEdges[3];
		looseEdgePtr3->PrevHalfEdge = BoundryEdges[4];

		BoundryEdges[3]->NextHalfEdge = BoundryEdges[4];
		BoundryEdges[3]->PrevHalfEdge = looseEdgePtr3;

		BoundryEdges[4]->NextHalfEdge = looseEdgePtr3;
		BoundryEdges[4]->PrevHalfEdge = BoundryEdges[3];


		//FACE CONNECTING
		BoundryEdges[0]->Face = newFace1;
		looseEdgePtr1->Face = newFace1;
		BoundryEdges[5]->Face = newFace1;

		looseEdgeTwinPtr1->Face = newFace2;
		looseEdgeTwinPtr2->Face = newFace2;
		looseEdgeTwinPtr3->Face = newFace2;

		BoundryEdges[1]->Face = newFace3;
		looseEdgePtr2->Face = newFace3;
		BoundryEdges[2]->Face = newFace3;

		looseEdgePtr3->Face = face;
		BoundryEdges[3]->Face = face;
		BoundryEdges[4]->Face = face;


		newFace1->halfEdge = BoundryEdges[0];
		newFace2->halfEdge = looseEdgeTwinPtr1;
		newFace3->halfEdge = BoundryEdges[1];
		face->halfEdge = looseEdgePtr3;

		unsigned int InputVertexIndex1 = face->Indices[0];
		unsigned int InputVertexIndex2 = face->Indices[1];
		unsigned int InputVertexIndex3 = face->Indices[2];

		newFace1->Indices.push_back(InputVertexIndex1);
		newFace1->Indices.push_back(NewVertexIndices[0]);
		newFace1->Indices.push_back(NewVertexIndices[2]);

		newFace2->Indices.push_back(NewVertexIndices[2]);
		newFace2->Indices.push_back(NewVertexIndices[0]);
		newFace2->Indices.push_back(NewVertexIndices[1]);

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

	std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, FUSIONCORE::PairVec3Hash>::iterator itr;
	for (itr = EdgeMap.begin(); itr != EdgeMap.end(); itr++)
	{
		auto twinPair = std::make_pair(itr->first.second, itr->first.first);
		if (EdgeMap.find(twinPair) == EdgeMap.end())
		{
			HalfEdges[itr->second]->BoundryEdge = true;
		}
		else
		{
			HalfEdges[itr->second]->TwinHalfEdge = HalfEdges[EdgeMap[twinPair]].get();
			HalfEdges[itr->second]->BoundryEdge = false;
		}
	}

	std::swap(ResultantIndices, Indicies);

	if (level > 1)
	{
		LoopSubdivision(Mesh, level - 1);
	}
	else
	{
		Mesh.ConstructMesh();
	}
}

void FUSIONPHYSICS::MESHOPERATIONS::CollapseDecimation(FUSIONCORE::Mesh& Mesh, int level)
{
}
