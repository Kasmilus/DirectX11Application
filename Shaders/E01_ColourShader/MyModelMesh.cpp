#include "MyModelMesh.h"
// load model data, initialise buffers (with model data) and load texture.
MyModelMesh::MyModelMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	loadModel(filename);
	initBuffers(device);
}

// Release resources.
MyModelMesh::~MyModelMesh()
{
	// Run parent deconstructor
	MyBaseMesh::~MyBaseMesh();

	if (model)
	{
		delete[] model;
		model = 0;
	}
}


// Initialise buffers with model data.
void MyModelMesh::initBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	// Load the vertex array and index array with data.
	for (int i = 0; i < vertexCount; i++)
	{
		vertices[i].position = XMFLOAT3(model[i].x, model[i].y, -model[i].z);
		vertices[i].texture = XMFLOAT2(model[i].tu, model[i].tv);
		vertices[i].normal = XMFLOAT3(model[i].nx, model[i].ny, -model[i].nz);
		vertices[i].tangent = XMFLOAT3(model[i].tx, model[i].ty, -model[i].tz);
		vertices[i].binormal = XMFLOAT3(model[i].bx, model[i].by, -model[i].bz);

		indices[i] = i;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType)* vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long)* indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
}

// Modified from a mulit-threaded version by Mark Ropper (CGT).
void MyModelMesh::loadModel(char* filename)
{
	std::vector<XMFLOAT3> verts;
	std::vector<XMFLOAT3> norms;
	std::vector<XMFLOAT2> texCs;
	std::vector<unsigned int> faces;

	FILE* file;// = fopen(filename, "r");
	errno_t err;
	err = fopen_s(&file, filename, "r");
	if (err != 0)
		//if (file == NULL)
	{
		return;
	}

	while (true)
	{
		char lineHeader[128];

		// Read first word of the line
		int res = fscanf_s(file, "%s", lineHeader, sizeof(lineHeader));
		if (res == EOF)
		{
			break; // exit loop
		}
		else // Parse
		{
			if (strcmp(lineHeader, "v") == 0) // Vertex
			{
				XMFLOAT3 vertex;
				fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				verts.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0) // Tex Coord
			{
				XMFLOAT2 uv;
				fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
				texCs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0) // Normal
			{
				XMFLOAT3 normal;
				fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				norms.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0) // Face
			{
				unsigned int face[9];
				int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &face[0], &face[1], &face[2],
					&face[3], &face[4], &face[5],
					&face[6], &face[7], &face[8]);
				if (matches != 9)
				{
					// Parser error, or not triangle faces
					return;
				}

				for (int i = 0; i < 9; i++)
				{
					faces.push_back(face[i]);
				}


			}
		}
	}

	int vIndex = 0, nIndex = 0, tIndex = 0;
	int numFaces = (int)faces.size() / 9;

	//// Create the model using the vertex count that was read in.
	vertexCount = numFaces * 3;
	model = new ModelType[vertexCount];

	// "Unroll" the loaded obj information into a list of triangles.
	for (int f = 0; f < (int)faces.size(); f += 3)
	{
		model[vIndex].x = verts[(faces[f + 0] - 1)].x;
		model[vIndex].y = verts[(faces[f + 0] - 1)].y;
		model[vIndex].z = verts[(faces[f + 0] - 1)].z;
		model[vIndex].tu = texCs[(faces[f + 1] - 1)].x;
		model[vIndex].tv = texCs[(faces[f + 1] - 1)].y;
		model[vIndex].nx = norms[(faces[f + 2] - 1)].x;
		model[vIndex].ny = norms[(faces[f + 2] - 1)].y;
		model[vIndex].nz = norms[(faces[f + 2] - 1)].z;

		//increase index count
		vIndex++;

	}
	indexCount = vIndex;

	// Now calculate tangent and binormal vectors
	XMFLOAT3 tangent, binormal;
	for (int i = 0; i < numFaces * 3; i += 3)
	{
		// Use three vertices of the traingle to calculate tangent and binormal
		calculateTangentAndBinormal(model[i], model[i + 1], model[i + 2], tangent, binormal);

		// Store tangent and binormal
		model[i].tx = tangent.x;
		model[i].ty = tangent.y;
		model[i].tz = tangent.z;
		model[i].bx = binormal.x;
		model[i].by = binormal.y;
		model[i].bz = binormal.z;

		model[i + 1].tx = tangent.x;
		model[i + 1].ty = tangent.y;
		model[i + 1].tz = tangent.z;
		model[i + 1].bx = binormal.x;
		model[i + 1].by = binormal.y;
		model[i + 1].bz = binormal.z;

		model[i + 2].tx = tangent.x;
		model[i + 2].ty = tangent.y;
		model[i + 2].tz = tangent.z;
		model[i + 2].bx = binormal.x;
		model[i + 2].by = binormal.y;
		model[i + 2].bz = binormal.z;
	}


	verts.clear();
	norms.clear();
	texCs.clear();
	faces.clear();

}

// Function calculating tangent based on [http://www.rastertek.com/dx11tut20.html]
void MyModelMesh::calculateTangentAndBinormal(ModelType vertex1, ModelType vertex2, ModelType vertex3, XMFLOAT3 & tangent, XMFLOAT3 & binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;

	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;
}