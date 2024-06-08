#include "CommonHeader.h"
#include "FShaderHelperStyle.h"
#include "Common/Path/BaseResourcePath.h"
#include <Styling/SlateStyleRegistry.h>
#include <Styling/StyleColors.h>
#include <Styling/SlateStyleMacros.h>

#define RootToContentDir Style->RootToContentDir
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo(RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )

using namespace FRAMEWORK;

namespace SH {

    const ISlateStyle& FShaderHelperStyle::Get()
    {
        if (!Instance.IsValid()) {
            Instance = Create();
            FSlateStyleRegistry::RegisterSlateStyle(Get());
        }
        return *Instance;
    }

	TSharedRef<ISlateStyle> FShaderHelperStyle::Create()
	{
		TSharedRef<FSlateStyleSet> Style = MakeShared<FSlateStyleSet>("ShaderHelperStyle");
        Style->SetContentRoot(BaseResourcePath::Custom_SlateResourceDir);
        Style->Set("AssetBrowser.ShaderPass", new IMAGE_BRUSH_SVG("ShaderPass", FVector2D(64.0, 64.0)));
        
		Style->SetContentRoot(BaseResourcePath::UE_SlateResourceDir);
        
        FButtonStyle SuperSimpleButton = FButtonStyle(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
            .SetPressed(FSlateNoResource())
            .SetHovered(FSlateNoResource());
        Style->Set("SuperSimpleButton", SuperSimpleButton);

		FTableRowStyle LineNumberItemStyle;
		LineNumberItemStyle.SetEvenRowBackgroundBrush(FSlateNoResource());
		LineNumberItemStyle.SetOddRowBackgroundBrush(FSlateNoResource());

		Style->Set("LineNumberItemStyle", LineNumberItemStyle);

		FTableRowStyle LineTipItemStyle;
		LineTipItemStyle.SetEvenRowBackgroundBrush(FSlateColorBrush{ FLinearColor::White});
		LineTipItemStyle.SetOddRowBackgroundBrush(FSlateColorBrush{ FLinearColor::White });
		
		Style->Set("LineTipItemStyle", LineTipItemStyle);

        TSharedRef<FCompositeFont> CodeFont = MakeShared<FStandaloneCompositeFont>();
        CodeFont->DefaultTypeface.AppendFont(TEXT("Code"), BaseResourcePath::UE_SlateFontDir / TEXT("DroidSansMono.ttf"), EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
        CodeFont->FallbackTypeface.Typeface.AppendFont(TEXT("Code"), BaseResourcePath::UE_SlateFontDir / TEXT("DroidSansFallback.ttf"), EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
        FSlateFontInfo CodeFontInfo = FSlateFontInfo(CodeFont, 10);
        Style->Set("CodeFont", CodeFontInfo);

		FTextBlockStyle CodeEditorNormalTextStyle = FTextBlockStyle{}
			.SetFont(CodeFontInfo)
			.SetColorAndOpacity(FLinearColor::White);
		
		FTextBlockStyle CodeEditorPunctuationTextStyle = FTextBlockStyle{ CodeEditorNormalTextStyle }
			.SetColorAndOpacity(FLinearColor{1.0f, 0.65f, 0.0f, 1.0f});

		FTextBlockStyle CodeEditorKeywordTextStyle = FTextBlockStyle{ CodeEditorNormalTextStyle }
			.SetColorAndOpacity(FLinearColor{1.0f, 0.4f, 0.7f, 1.0f});

		FTextBlockStyle CodeEditorNumberTextStyle = FTextBlockStyle{ CodeEditorNormalTextStyle }
			.SetColorAndOpacity(FLinearColor{ 0.5f, 1.0f, 0.66f, 1.0f });
			
		FTextBlockStyle CodeEditorCommentTextStyle = FTextBlockStyle{ CodeEditorNormalTextStyle }
			.SetColorAndOpacity(FLinearColor{0.2f, 0.9f, 0.2f, 1.0f});

		FTextBlockStyle CodeEditorBuildtinFuncTextStyle = FTextBlockStyle{ CodeEditorNormalTextStyle }
			.SetColorAndOpacity(FLinearColor{0.58f, 0.43f, 0.82f,1.0f});

		FTextBlockStyle CodeEditorBuildtinTypeTextStyle = FTextBlockStyle{ CodeEditorNormalTextStyle }
			.SetColorAndOpacity(FLinearColor{ 0.1f, 0.55f, 1.0f, 1.0f });

		FTextBlockStyle CodeEditorPreprocessTextStyle = FTextBlockStyle{ CodeEditorNormalTextStyle }
			.SetColorAndOpacity(FLinearColor{0.35f, 0.35f, 0.35f, 1.0f});

		Style->Set("CodeEditorNormalText", CodeEditorNormalTextStyle);
		Style->Set("CodeEditorPunctuationText", CodeEditorPunctuationTextStyle);
		Style->Set("CodeEditorKeywordText", CodeEditorKeywordTextStyle);
		Style->Set("CodeEditorNumberText", CodeEditorNumberTextStyle);
		Style->Set("CodeEditorCommentText", CodeEditorCommentTextStyle);
		Style->Set("CodeEditorBuildtinFuncText", CodeEditorBuildtinFuncTextStyle);
		Style->Set("CodeEditorBuildtinTypeText", CodeEditorBuildtinTypeTextStyle);
		Style->Set("CodeEditorPreprocessText", CodeEditorPreprocessTextStyle);

		FTextBlockStyle CodeEditorErrorInfoStyle = FTextBlockStyle{ CodeEditorNormalTextStyle }
			.SetColorAndOpacity(FLinearColor::Red);
		Style->Set("CodeEditorErrorInfoText", CodeEditorErrorInfoStyle);
		
		const FVector2D Icon14x14(14.0f, 14.0f);
		
		const FButtonStyle ArrowDownButton = FButtonStyle()
			.SetNormal(IMAGE_BRUSH("Extra/ArrowDown", Icon14x14))
			.SetHovered(IMAGE_BRUSH("Extra/ArrowDown", Icon14x14))
			.SetPressed(IMAGE_BRUSH("Extra/ArrowDown", Icon14x14));

		const FButtonStyle ArrowRightButton = FButtonStyle()
			.SetNormal(IMAGE_BRUSH("Extra/ArrowRight", Icon14x14))
			.SetHovered(IMAGE_BRUSH("Extra/ArrowRight", Icon14x14))
			.SetPressed(IMAGE_BRUSH("Extra/ArrowRight", Icon14x14));

		Style->Set("ArrowDownButton", ArrowDownButton);
		Style->Set("ArrowRightButton", ArrowRightButton);

		Style->Set("Icons.World", new IMAGE_BRUSH_SVG("Starship/Common/world", FVector2D{16.0f,16.0f}));
		return Style;
	}

}

#undef TTF_FONT
#undef RootToContentDir
