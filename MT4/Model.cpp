#include "Model.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// 静的メンバ変数の実体 The entity of a static member variable
const std::string Model::baseDirectory = "Resources/";
ID3D12Device* Model::device = nullptr;
UINT Model::descriptorHandleIncrementSize = 0;

void Model::StaticInitialize(ID3D12Device * device)
{
	Model::device = device;

	// メッシュの静的初期化 Static initialization of mesh
	Mesh::StaticInitialize(device);
}

Model* Model::CreateFromOBJ(const std::string& modelname)
{
	// メモリ確保 Memory allocation
	Model* instance = new Model;
	instance->Initialize(modelname);
	
	return instance;
}

Model::~Model()
{
	for (auto m : meshes) {
		delete m;
	}
	meshes.clear();

	for (auto m : materials) {
		delete m.second;
	}
	materials.clear();
}

void Model::Initialize(const std::string& modelname)
{
	const string filename = modelname + ".obj";
	const string directoryPath = baseDirectory + modelname + "/";

	// ファイルストリーム File stream
	std::ifstream file;
	// .objファイルを開く Open .obj file
	file.open(directoryPath + filename);
	// ファイルオープン失敗をチェック Check file open failure
	if (file.fail()) {
		assert(0);
	}

	name = modelname;

	// メッシュ生成 Mesh generation
	Mesh* mesh = new Mesh;
	int indexCountTex = 0;
	int indexCountNoTex = 0;

	vector<XMFLOAT3> positions;	// 頂点座標 Vertex coordinates
	vector<XMFLOAT3> normals;	// 法線ベクトル Normal vector
	vector<XMFLOAT2> texcoords;	// テクスチャUV Texture UV
	// 1行ずつ読み込む Read line by line
	string line;
	while (getline(file, line)) {

		// 1行分の文字列をストリームに変換して解析しやすくする Convert one line of string to a stream to make it easier to parse
		std::istringstream line_stream(line);

		// 半角スペース区切りで行の先頭文字列を取得 Get the first character string of a line separated by a single-byte space
		string key;
		getline(line_stream, key, ' ');

		//マテリアル material
		if (key == "mtllib")
		{
			// マテリアルのファイル名読み込み Read material file name
			string filename;
			line_stream >> filename;
			// マテリアル読み込み Material loading
			LoadMaterial(directoryPath, filename);
		}
		// 先頭文字列がgならグループの開始 If the first string is g, the start of the group
		if (key == "g") {
            // カレントメッシュの情報が揃っているなら  If you have all the information about the current mesh
            if (mesh->GetName().size() > 0 && mesh->GetVertexCount() > 0) {
				// コンテナに登録 Register in container
				meshes.emplace_back(mesh);
				// 次のメッシュ生成 Next mesh generation
				mesh = new Mesh;
				indexCountTex = 0;
			}

			// グループ名読み込み Read group name
			string groupName;
			line_stream >> groupName;

			// メッシュに名前をセット Set the name on the mesh
			mesh->SetName(groupName);
		}
		// 先頭文字列がvなら頂点座標 Vertex coordinates if the first string is v
		if (key == "v") {
			// X,Y,Z座標読み込み Read X, Y, Z coordinates
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			positions.emplace_back(position);
		}
		// 先頭文字列がvtならテクスチャ Texture if the first string is vt
		if (key == "vt")
		{
			// U,V成分読み込み U, V component reading
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			// V方向反転 V direction inversion
			texcoord.y = 1.0f - texcoord.y;
			// テクスチャ座標データに追加 Add to texture coordinate data
			texcoords.emplace_back(texcoord);
		}
		// 先頭文字列がvnなら法線ベクトル Normal vector if the first string is vn
		if (key == "vn") {
			// X,Y,Z成分読み込み Read X, Y, Z components
			XMFLOAT3 normal{};
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			// 法線ベクトルデータに追加 Added to normal vector data
			normals.emplace_back(normal);
		}
		// 先頭文字列がusemtlならマテリアルを割り当てる Assign material if the first string is usemtl
		if (key == "usemtl")
		{
			if (mesh->GetMaterial() == nullptr) {
				// マテリアルの名読み込み Material name reading
				string materialName;
				line_stream >> materialName;

				// マテリアル名で検索し、マテリアルを割り当てる Search by material name and assign a material
				auto itr = materials.find(materialName);
				if (itr != materials.end()) {
					mesh->SetMaterial(itr->second);
				}
			}
		}
		// 先頭文字列がfならポリゴン（三角形） If the first character string is f, it is a polygon (triangle)
		if (key == "f")
		{
			int faceIndexCount = 0;
			// 半角スペース区切りで行の続きを読み込む Read the continuation of the line separated by a single-byte space
			string index_string;
			while (getline(line_stream, index_string, ' ')) {
				// 頂点インデックス1個分の文字列をストリームに変換して解析しやすくする Converts a character string for one vertex index into a stream to make it easier to analyze
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;
				// 頂点番号 Vertex number
				index_stream >> indexPosition;				

				Material* material = mesh->GetMaterial();
				index_stream.seekg(1, ios_base::cur); // スラッシュを飛ばす Skip the slash
				// マテリアル、テクスチャがある場合 If there are materials and textures
				if (material && material->textureFilename.size() > 0) {
					index_stream >> indexTexcoord;
					index_stream.seekg(1, ios_base::cur); // スラッシュを飛ばす Skip the slash
					index_stream >> indexNormal;
					// 頂点データの追加 Add vertex data
					Mesh::VertexPosNormalUv vertex{};
					vertex.pos = positions[indexPosition - 1];
					vertex.normal = normals[indexNormal - 1];
					vertex.uv = texcoords[indexTexcoord - 1];
					mesh->AddVertex(vertex);					
				}
				else {
					char c;
					index_stream >> c;
					// スラッシュ2連続の場合、頂点番号のみ For two consecutive slashes, only the vertex number
					if (c == '/') {
						// 頂点データの追加 Add vertex data
						Mesh::VertexPosNormalUv vertex{};
						vertex.pos = positions[indexPosition - 1];
						vertex.normal = { 0, 0, 1 };
						vertex.uv = { 0, 0 };
						mesh->AddVertex(vertex);
					}
					else {
						index_stream.seekg(-1, ios_base::cur); // 1文字戻る Back one character
						index_stream >> indexTexcoord;
						index_stream.seekg(1, ios_base::cur); // スラッシュを飛ばす Skip the slash
						index_stream >> indexNormal;
						// 頂点データの追加 Add vertex data
						Mesh::VertexPosNormalUv vertex{};
						vertex.pos = positions[indexPosition - 1];
						vertex.normal = normals[indexNormal - 1];
						vertex.uv = { 0, 0 };
						mesh->AddVertex(vertex);
					}
				}
				// インデックスデータの追加 Add index data
				if (faceIndexCount >= 3) {
					// 四角形ポリゴンの4点目なので、 Because it is the 4th point of the quadrilateral polygon
					// 四角形の0,1,2,3の内 2,3,0で三角形を構築する Build a triangle with 2,3,0 out of 0,1,2,3 of a quadrangle
					mesh->AddIndex(indexCountTex - 1);
					mesh->AddIndex(indexCountTex);
					mesh->AddIndex(indexCountTex - 3);
				}
				else
				{
					mesh->AddIndex(indexCountTex);
				}
				indexCountTex++;
				faceIndexCount++;
			}
		}
	}
	file.close();

	// コンテナに登録 Register in container
	meshes.emplace_back(mesh);

	//// １つもマテリアルがなければ If there is no material
	//if (materials.size() == 0) {
	//	// 生成して追加する
	//	Material* material = Material::Create();
	//	material->name = "no material";
	//	materials.emplace(material->name, material);

	//	// 全メッシュに適用 Applies to all meshes
	//	for_each(meshes.begin(), meshes.end(), [&](Mesh* m) {m->SetMaterial(material); });
	//}

	// メッシュのマテリアルチェック Mesh material check
	for (auto& m : meshes) {
		// マテリアルの割り当てがない No material assignment
		if (m->GetMaterial() == nullptr) {
			if (defaultMaterial == nullptr) {
				// デフォルトマテリアルを生成 Generate default material
				defaultMaterial = Material::Create();
				defaultMaterial->name = "no material";
				materials.emplace(defaultMaterial->name, defaultMaterial);
			}
			// デフォルトマテリアルをセット Set default material
			m->SetMaterial(defaultMaterial);
		}
	}

	// メッシュのバッファ生成 Mesh buffer generation
	for (auto& m : meshes) {
		m->CreateBuffers();
	}

	// マテリアルの数値を定数バッファに反映 Reflect the numerical value of the material in the constant buffer
	for (auto& m : materials) {
		m.second->Update();
	}

	// デスクリプタヒープ生成 Descriptor heap generation
	CreateDescriptorHeap();

	// テクスチャの読み込み Loading texture
	LoadTextures();
}

void Model::LoadMaterial(const std::string & directoryPath, const std::string & filename)
{
	// ファイルストリーム File stream
	std::ifstream file;
	// マテリアルファイルを開く Open material file
	file.open(directoryPath + filename);
	// ファイルオープン失敗をチェック Check file open failure
	if (file.fail()) {
		assert(0);
	}

	Material* material = nullptr;

	// 1行ずつ読み込む Read line by line
	string line;
	while (getline(file, line)) {

		// 1行分の文字列をストリームに変換して解析しやすくする Convert one line of string to a stream to make it easier to parse
		std::istringstream line_stream(line);

		// 半角スペース区切りで行の先頭文字列を取得 Get the first character string of a line separated by a single-byte space
		string key;
		getline(line_stream, key, ' ');

		// 先頭のタブ文字は無視する Ignore the first tab character
		if (key[0] == '\t') {
			key.erase(key.begin()); // 先頭の文字を削除 Remove the first character
		}

		// 先頭文字列がnewmtlならマテリアル名 Material name if the first string is newmtl
		if (key == "newmtl") {

			// 既にマテリアルがあれば If you already have the material
			if (material) {
				// マテリアルをコンテナに登録 Register the material in the container
				AddMaterial(material);
			}

			// 新しいマテリアルを生成 Generate new material
			material = Material::Create();
			// マテリアル名読み込み Material name read
			line_stream >> material->name;
		}
		// 先頭文字列がKaならアンビエント色 Ambient color if the first string is Ka
		if (key == "Ka") {
			line_stream >> material->ambient.x;
			line_stream >> material->ambient.y;
			line_stream >> material->ambient.z;
		}
		// 先頭文字列がKdならディフューズ色 If the first character string is Kd, the diffuse color
		if (key == "Kd") {
			line_stream >> material->diffuse.x;
			line_stream >> material->diffuse.y;
			line_stream >> material->diffuse.z;
		}
		// 先頭文字列がKsならスペキュラー色 Specular color if the first string is Ks
		if (key == "Ks") {
			line_stream >> material->specular.x;
			line_stream >> material->specular.y;
			line_stream >> material->specular.z;
		}
		// 先頭文字列がmap_Kdならテクスチャファイル名 Texture file name if the first string is map_Kd
		if (key == "map_Kd") {
			// テクスチャのファイル名読み込み Read texture file name
			line_stream >> material->textureFilename;

			// フルパスからファイル名を取り出す Extract the file name from the full path
			size_t pos1;
			pos1 = material->textureFilename.rfind('\\');
			if (pos1 != string::npos) {
				material->textureFilename = material->textureFilename.substr(pos1 + 1, material->textureFilename.size() - pos1 - 1);
			}

			pos1 = material->textureFilename.rfind('/');
			if (pos1 != string::npos) {
				material->textureFilename = material->textureFilename.substr(pos1 + 1, material->textureFilename.size() - pos1 - 1);
			}
		}
	}
	// ファイルを閉じる Close file
	file.close();	

	if (material) {
		// マテリアルを登録 Register material
		AddMaterial(material);
	}
}

void Model::AddMaterial(Material * material)
{
	// コンテナに登録 Register in container
	materials.emplace(material->name, material);
}

void Model::CreateDescriptorHeap()
{
	HRESULT result = S_FALSE;

	// マテリアルの数 Number of materials
	size_t count = materials.size();

	// デスクリプタヒープを生成	 Generate descriptor heap
	if (count > 0) {
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように As visible from the shader
		descHeapDesc.NumDescriptors = (UINT)count; // シェーダーリソースビューの数 Number of shader resource views
		result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));//生成 generation
		if (FAILED(result)) {
			assert(0);
		}
	}

	// デスクリプタサイズを取得 Get descriptor size
	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Model::LoadTextures()
{
	int textureIndex = 0;
	string directoryPath = baseDirectory + name + "/";

	for (auto& m : materials) {
		Material* material = m.second;

		// テクスチャあり With texture
		if (material->textureFilename.size() > 0) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), textureIndex, descriptorHandleIncrementSize);
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), textureIndex, descriptorHandleIncrementSize);
			// マテリアルにテクスチャ読み込み Import texture into material
			material->LoadTexture(directoryPath, cpuDescHandleSRV, gpuDescHandleSRV);

			textureIndex++;
		}
		// テクスチャなしNo texture
		else {
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), textureIndex, descriptorHandleIncrementSize);
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), textureIndex, descriptorHandleIncrementSize);
			// マテリアルにテクスチャ読み込み Import texture into material
			material->LoadTexture(baseDirectory, cpuDescHandleSRV, gpuDescHandleSRV);

			textureIndex++;
		}
	}
}

void Model::Draw(ID3D12GraphicsCommandList * cmdList)
{
	// デスクリプタヒープの配列 Array of descriptor heaps
	if (descHeap) {
		ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
		cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}

	// 全メッシュを描画 Draw all meshes
	for (auto& mesh : meshes) {
		mesh->Draw(cmdList);
	}
}

