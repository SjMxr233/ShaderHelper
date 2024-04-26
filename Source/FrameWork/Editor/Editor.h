#pragma once

namespace FRAMEWORK {

	enum class SupportedLanguage
	{
		English,
		Chinese,
		Num,
	};

	class FRAMEWORK_API Editor
	{
	public:
		Editor();
		virtual ~Editor();

		static void SetLanguage(SupportedLanguage InLanguage);
		static inline SupportedLanguage CurLanguage = SupportedLanguage::English;
		static inline TUniquePtr<FConfigFile> EditorConfig = MakeUnique<FConfigFile>();
	};
}


