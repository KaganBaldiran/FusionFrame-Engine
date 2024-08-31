#pragma once
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include <map>
#include "../FusionUtility/FusionDLLExport.h"

namespace FUSIONCORE
{

#define COLORS_IMP

#ifdef COLORS_IMP

    class FUSIONFRAME_EXPORT Color
	{
	public:

		Color() = default;
		Color(glm::vec4 Value);

		void SetRGBA(glm::vec4 Value);
		void SetRGB(glm::vec3 Value);
		void SetRed(float Value);
		void SetBlue(float Value);
		void SetGreen(float Value);
		void SetAlpha(float Value);

		void Brighter(float IncrementBy = 0.1f);
		void Darker(float DecrementBy = 0.1f);

		glm::vec3 GetRGB();
		float GetRed();
		float GetBlue();
		float GetGreen();
		float GetAlpha();
		glm::vec4 GetRBGA();
		glm::vec4 GetRGBA();

	private:
		glm::vec4 Value;
	};


#define FF_COLOR_RED glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
#define FF_COLOR_VOID glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)
#define FF_COLOR_GREEN glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
#define FF_COLOR_BLUE glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
#define FF_COLOR_WHITE glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
#define FF_COLOR_BLACK glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
#define FF_COLOR_YELLOW glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)
#define FF_COLOR_MAGENTA glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)
#define FF_COLOR_CYAN glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)
#define FF_COLOR_ORANGE glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)
#define FF_COLOR_PURPLE glm::vec4(0.5f, 0.0f, 0.5f, 1.0f)
#define FF_COLOR_PINK glm::vec4(1.0f, 0.7f, 0.7f, 1.0f)
#define FF_COLOR_LIME glm::vec4(0.7f, 1.0f, 0.7f, 1.0f)
#define FF_COLOR_BROWN glm::vec4(0.6f, 0.3f, 0.1f, 1.0f)
#define FF_COLOR_GRAY glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
#define FF_COLOR_LIGHT_BLUE glm::vec4(0.7f, 0.7f, 1.0f, 1.0f)
#define FF_COLOR_DARK_GREEN glm::vec4(0.0f, 0.5f, 0.0f, 1.0f)
#define FF_COLOR_SKY_BLUE glm::vec4(0.4f, 0.8f, 1.0f, 1.0f)
#define FF_COLOR_TAN glm::vec4(0.8f, 0.6f, 0.4f, 1.0f)
#define FF_COLOR_GOLD glm::vec4(1.0f, 0.8f, 0.2f, 1.0f)
#define FF_COLOR_SILVER glm::vec4(0.8f, 0.8f, 0.8f, 1.0f)
#define FF_COLOR_VIOLET glm::vec4(0.5f, 0.0f, 1.0f, 1.0f)
#define FF_COLOR_LAVENDER glm::vec4(0.7f, 0.5f, 1.0f, 1.0f)
#define FF_COLOR_MAROON glm::vec4(0.5f, 0.0f, 0.0f, 1.0f)
#define FF_COLOR_OLIVE glm::vec4(0.5f, 0.5f, 0.0f, 1.0f)
#define FF_COLOR_TEAL glm::vec4(0.0f, 0.5f, 0.5f, 1.0f)
#define FF_COLOR_CORAL glm::vec4(1.0f, 0.5f, 0.3f, 1.0f)
#define FF_COLOR_INDIGO glm::vec4(0.3f, 0.0f, 0.7f, 1.0f)
#define FF_COLOR_SALMON glm::vec4(1.0f, 0.6f, 0.6f, 1.0f)
#define FF_COLOR_KHAKI glm::vec4(0.9f, 0.9f, 0.5f, 1.0f)
#define FF_COLOR_NAVY_BLUE glm::vec4(0.0f, 0.0f, 0.5f, 1.0f)
#define FF_COLOR_PEACH glm::vec4(1.0f, 0.8f, 0.6f, 1.0f)
#define FF_COLOR_SEAFOAM glm::vec4(0.4f, 1.0f, 0.7f, 1.0f)
#define FF_COLOR_PLUM glm::vec4(0.5f, 0.2f, 0.5f, 1.0f)
#define FF_COLOR_BRICK_RED glm::vec4(0.8f, 0.2f, 0.2f, 1.0f)
#define FF_COLOR_STEEL_GRAY glm::vec4(0.6f, 0.7f, 0.8f, 1.0f)
#define FF_COLOR_MAUVE glm::vec4(0.8f, 0.6f, 0.8f, 1.0f)
#define FF_COLOR_SAGE_GREEN glm::vec4(0.5f, 0.7f, 0.5f, 1.0f)
#define FF_COLOR_ROSE glm::vec4(1.0f, 0.4f, 0.7f, 1.0f)
#define FF_COLOR_BRONZE glm::vec4(0.8f, 0.4f, 0.2f, 1.0f)
#define FF_COLOR_CHARCOAL glm::vec4(0.3f, 0.3f, 0.3f, 1.0f)
#define FF_COLOR_MINT_GREEN glm::vec4(0.7f, 1.0f, 0.7f, 1.0f)
#define FF_COLOR_LILAC glm::vec4(0.8f, 0.6f, 1.0f, 1.0f)
#define FF_COLOR_GOLDENROD glm::vec4(0.8f, 0.7f, 0.2f, 1.0f)
#define FF_COLOR_CYBER_YELLOW glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)
#define FF_COLOR_OCEAN_BLUE glm::vec4(0.0f, 0.4f, 0.8f, 1.0f)
#define FF_COLOR_RUBY_RED glm::vec4(0.8f, 0.0f, 0.2f, 1.0f)
#define FF_COLOR_TURQUOISE glm::vec4(0.2f, 0.8f, 0.7f, 1.0f)
#define FF_COLOR_EGGPLANT glm::vec4(0.3f, 0.0f, 0.4f, 1.0f)
#define FF_COLOR_CINNAMON glm::vec4(0.8f, 0.4f, 0.2f, 1.0f)
#define FF_COLOR_JADE_GREEN glm::vec4(0.3f, 0.8f, 0.5f, 1.0f)
#define FF_COLOR_PERSIMMON glm::vec4(1.0f, 0.4f, 0.2f, 1.0f)
#define FF_COLOR_AMETHYST glm::vec4(0.6f, 0.4f, 0.8f, 1.0f)
#define FF_COLOR_LEMONADE glm::vec4(1.0f, 1.0f, 0.5f, 1.0f)
#define FF_COLOR_PEARL_WHITE glm::vec4(0.9f, 0.9f, 0.95f, 1.0f)
#define FF_COLOR_SUNKISSED_ORANGE glm::vec4(1.0f, 0.6f, 0.2f, 1.0f)
#define FF_COLOR_MANGO_TANGO glm::vec4(1.0f, 0.5f, 0.2f, 1.0f)
#define FF_COLOR_LAGOON_BLUE glm::vec4(0.2f, 0.6f, 0.8f, 1.0f)
#define FF_COLOR_RUSTIC_BROWN glm::vec4(0.5f, 0.3f, 0.2f, 1.0f)
#define FF_COLOR_LILY_PINK glm::vec4(1.0f, 0.7f, 0.8f, 1.0f)
#define FF_COLOR_OBSIDIAN_BLACK glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
#define FF_COLOR_SUNFLOWER_YELLOW glm::vec4(1.0f, 0.9f, 0.2f, 1.0f)
#define FF_COLOR_AQUAMARINE glm::vec4(0.5f, 1.0f, 0.8f, 1.0f)
#define FF_COLOR_CRIMSON_RED glm::vec4(0.9f, 0.1f, 0.2f, 1.0f)
#define FF_COLOR_PEACOCK_BLUE glm::vec4(0.0f, 0.4f, 0.6f, 1.0f)
#define FF_COLOR_DUSK_PURPLE glm::vec4(0.4f, 0.2f, 0.6f, 1.0f)
#define FF_COLOR_CORNFLOWER_BLUE glm::vec4(0.4f, 0.6f, 0.9f, 1.0f)
#define FF_COLOR_TIGER_ORANGE glm::vec4(1.0f, 0.5f, 0.2f, 1.0f)
#define FF_COLOR_MISTY_ROSE glm::vec4(1.0f, 0.8f, 0.8f, 1.0f)
#define FF_COLOR_VERMILION_RED glm::vec4(0.8f, 0.1f, 0.2f, 1.0f)
#define FF_COLOR_JUNGLE_GREEN glm::vec4(0.1f, 0.5f, 0.2f, 1.0f)
#define FF_COLOR_MANDARIN_ORANGE glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)
#define FF_COLOR_LAVENDER_BLUSH glm::vec4(1.0f, 0.9f, 0.95f, 1.0f)
#define FF_COLOR_MALACHITE_GREEN glm::vec4(0.2f, 0.8f, 0.4f, 1.0f)
#define FF_COLOR_COTTON_CANDY_PINK glm::vec4(1.0f, 0.7f, 0.9f, 1.0f)
#define FF_COLOR_BURNT_SIENNA glm::vec4(0.6f, 0.2f, 0.2f, 1.0f)
#define FF_COLOR_MINT_CREAM glm::vec4(0.9f, 1.0f, 0.9f, 1.0f)
#define FF_COLOR_PUMPKIN_ORANGE glm::vec4(1.0f, 0.4f, 0.2f, 1.0f)
#define FF_COLOR_STORMY_GRAY glm::vec4(0.4f, 0.4f, 0.4f, 1.0f)
#define FF_COLOR_LEMON_LIME glm::vec4(0.8f, 1.0f, 0.2f, 1.0f)
#define FF_COLOR_BABY_BLUE glm::vec4(0.7f, 0.7f, 1.0f, 1.0f)
#define FF_COLOR_RASPBERRY_RED glm::vec4(0.8f, 0.0f, 0.3f, 1.0f)
#define FF_COLOR_ROYAL_PURPLE glm::vec4(0.4f, 0.2f, 0.8f, 1.0f)
#define FF_COLOR_TURKISH_BLUE glm::vec4(0.0f, 0.5f, 0.7f, 1.0f)
#define FF_COLOR_LIMEADE_GREEN glm::vec4(0.6f, 0.8f, 0.2f, 1.0f)
#define FF_COLOR_TANGERINE_ORANGE glm::vec4(1.0f, 0.6f, 0.2f, 1.0f)
#define FF_COLOR_CANDY_APPLE_RED glm::vec4(1.0f, 0.0f, 0.1f, 1.0f)
#define FF_COLOR_WISTERIA_PURPLE glm::vec4(0.7f, 0.5f, 0.9f, 1.0f)
#define FF_COLOR_OYSTER_GRAY glm::vec4(0.9f, 0.9f, 0.8f, 1.0f)
#define FF_COLOR_OYSTER_GRAY glm::vec4(0.9f, 0.9f, 0.8f, 1.0f)
#define FF_COLOR_EARTH_BROWN glm::vec4(0.4f, 0.2f, 0.1f, 1.0f)
#define FF_COLOR_VANILLA_CREAM glm::vec4(1.0f, 0.9f, 0.8f, 1.0f)
#define FF_COLOR_RUST_ORANGE glm::vec4(0.8f, 0.3f, 0.1f, 1.0f)
#define FF_COLOR_AUBERGINE_PURPLE glm::vec4(0.3f, 0.1f, 0.2f, 1.0f)
#define FF_COLOR_PERSIAN_GREEN glm::vec4(0.0f, 0.6f, 0.4f, 1.0f)
#define FF_COLOR_COTTON_WHITE glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
#define FF_COLOR_PAPAYA_ORANGE glm::vec4(1.0f, 0.6f, 0.4f, 1.0f)
#define FF_COLOR_PERIWINKLE_BLUE glm::vec4(0.7f, 0.7f, 1.0f, 1.0f)
#define FF_COLOR_BRONZE_GOLD glm::vec4(0.8f, 0.5f, 0.2f, 1.0f)
#define FF_COLOR_LAVENDER_MIST glm::vec4(0.9f, 0.8f, 1.0f, 1.0f)
#define FF_COLOR_GUNMETAL_GRAY glm::vec4(0.2f, 0.2f, 0.3f, 1.0f)
#define FF_COLOR_BLUSH_PINK glm::vec4(1.0f, 0.8f, 0.9f, 1.0f)
#define FF_COLOR_AZURE glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)
#define FF_COLOR_MARIGOLD glm::vec4(1.0f, 0.7f, 0.2f, 1.0f)
#define FF_COLOR_EMERALD_GREEN glm::vec4(0.0f, 1.0f, 0.5f, 1.0f)
#define FF_COLOR_PEONY_PINK glm::vec4(1.0f, 0.4f, 0.6f, 1.0f)
#define FF_COLOR_TITANIUM_GRAY glm::vec4(0.7f, 0.7f, 0.7f, 1.0f)
#define FF_COLOR_CITRUS_YELLOW glm::vec4(1.0f, 0.9f, 0.1f, 1.0f)
#define FF_COLOR_MIDNIGHT_BLUE glm::vec4(0.0f, 0.0f, 0.5f, 1.0f)
#define FF_COLOR_CRIMSON_ROSE glm::vec4(0.9f, 0.2f, 0.4f, 1.0f)
#define FF_COLOR_TURQUOISE_GREEN glm::vec4(0.2f, 0.8f, 0.6f, 1.0f)
#define FF_COLOR_LILAC_BLOSSOM glm::vec4(0.8f, 0.6f, 0.9f, 1.0f)
#define FF_COLOR_STARDUST_SILVER glm::vec4(0.8f, 0.8f, 0.9f, 1.0f)
#define FF_COLOR_CORAL_REEF glm::vec4(1.0f, 0.5f, 0.4f, 1.0f)
#define FF_COLOR_AMBER_YELLOW glm::vec4(1.0f, 0.7f, 0.2f, 1.0f)
#define FF_COLOR_MYSTIC_MAUVE glm::vec4(0.6f, 0.4f, 0.7f, 1.0f)
#define FF_COLOR_LIME_SHERBET glm::vec4(0.8f, 1.0f, 0.4f, 1.0f)
#define FF_COLOR_PEACHY_KEEN glm::vec4(1.0f, 0.7f, 0.5f, 1.0f)
#define FF_COLOR_SERENE_SKY glm::vec4(0.5f, 0.7f, 1.0f, 1.0f)
#define FF_COLOR_SUNSET_ORANGE glm::vec4(1.0f, 0.4f, 0.1f, 1.0f)
#define FF_COLOR_IRIS_PURPLE glm::vec4(0.5f, 0.2f, 0.7f, 1.0f)
#define FF_COLOR_OLIVE_BRANCH glm::vec4(0.4f, 0.5f, 0.2f, 1.0f)
#define FF_COLOR_PEACH_BLOSSOM glm::vec4(1.0f, 0.8f, 0.6f, 1.0f)
#define FF_COLOR_AQUA_MARINE glm::vec4(0.2f, 0.8f, 0.7f, 1.0f)
#define FF_COLOR_WILLOW_GREEN glm::vec4(0.5f, 0.7f, 0.4f, 1.0f)
#define FF_COLOR_STARRY_NIGHT glm::vec4(0.1f, 0.1f, 0.3f, 1.0f)
#define FF_COLOR_RUBY_REDVELVET glm::vec4(0.8f, 0.1f, 0.3f, 1.0f)
#define FF_COLOR_BRILLIANT_TURQUOISE glm::vec4(0.1f, 0.9f, 0.9f, 1.0f)
#define FF_COLOR_FROSTY_VIOLET glm::vec4(0.7f, 0.5f, 0.8f, 1.0f)
#define FF_COLOR_COPPER_PENNY glm::vec4(0.7f, 0.3f, 0.2f, 1.0f)
#define FF_COLOR_CHAMPAGNE_ROSE glm::vec4(0.9f, 0.7f, 0.8f, 1.0f)
#define FF_COLOR_CANDY_CORN glm::vec4(1.0f, 0.8f, 0.2f, 1.0f)
#define FF_COLOR_WARM_COCOA glm::vec4(0.4f, 0.2f, 0.0f, 1.0f)
#define FF_COLOR_OCEAN_MIST glm::vec4(0.7f, 0.9f, 1.0f, 1.0f)
#define FF_COLOR_SWEET_LAVENDER glm::vec4(0.8f, 0.6f, 0.9f, 1.0f)
#define FF_COLOR_GOLDEN_SUNSET glm::vec4(1.0f, 0.6f, 0.2f, 1.0f)
#define FF_COLOR_SILVER_MOONLIGHT glm::vec4(0.8f, 0.8f, 0.9f, 1.0f)
#define FF_COLOR_CINNAMON_SPICE glm::vec4(0.8f, 0.4f, 0.2f, 1.0f)
#define FF_COLOR_LUSH_GREEN glm::vec4(0.2f, 0.7f, 0.4f, 1.0f)
#define FF_COLOR_ENCHANTED_LILAC glm::vec4(0.7f, 0.5f, 0.8f, 1.0f)
#define FF_COLOR_GLOWING_AMBER glm::vec4(1.0f, 0.6f, 0.2f, 1.0f)
#define FF_COLOR_COOL_SMOKE glm::vec4(0.6f, 0.6f, 0.7f, 1.0f)


#endif // COLORS_IMP
}