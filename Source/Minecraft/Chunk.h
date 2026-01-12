#pragma once
#include <array>

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Chunk.generated.h"


enum BlockType
{
	Empty = 0,      // Полностью пусто (не рисуется)
	Air = 1,        // Воздух (прозрачный, но может быть для логики)
	Grass = 2,      // Трава
	Dirt = 3,       // Земля
	Stone = 4,      // Камень
	Wood = 5,       // Дерево
	Leaves = 6,     // Листья
	Sand = 7,       // Песок
	Gravel = 8,     // Гравий
	Cobblestone = 9,// Булыжник
	Bricks = 10,    // Кирпичи
	Glass = 11,     // Стекло
	Water = 12,     // Вода
	Lava = 13,      // Лава
	Bedrock = 14,   // Бе́дрок
	IronBlock = 15, // Железный блок
	GoldBlock = 16, // Золотой блок
    
	// Для удобства
	Count = 17      // Количество типов блоков
};

CONSTEXPR int BLOCK_SIZE = 256.0f;
constexpr uint8_t CHUNK_SIZE = 64;
constexpr uint16_t CHUNK_Z = 256;
constexpr uint8_t MIN_HEIGHT = 20;
constexpr uint8_t MAX_HEIGHT = 96;
constexpr uint8_t WATER_LEVEL = 62;



UCLASS()
class MINECRAFT_API UChunk : public UObject
{
	GENERATED_BODY()
private:
	static constexpr int SLICE_SIZE = CHUNK_SIZE * CHUNK_SIZE;	
	static constexpr int TOTAL_BLOCKS = SLICE_SIZE  * CHUNK_Z;
	std::array<BlockType,TOTAL_BLOCKS> Cubes;
	std::array<uint16_t,SLICE_SIZE> Surface;	
	std::array<BlockType,TOTAL_BLOCKS> Chunks;
	inline int Index(uint8_t x, uint8_t y, uint16_t z) const;
	inline int SurfaceIndex(uint8_t x, uint8_t y) const;
public:
	void SetBlock(uint8_t x,uint8_t y,uint16_t z,BlockType type);
	BlockType GetBlock(uint8_t x, uint8_t y, uint16_t z) const;
	void SetSurfaceHeight(uint8_t x,uint8_t y,uint16_t height);
	void Fill();
};
