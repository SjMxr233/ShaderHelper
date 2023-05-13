#pragma once
#include <Framework/Text/BaseTextLayoutMarshaller.h>
#include <Widgets/Views/SListView.h>

namespace SH
{
	class FShaderEditorMarshaller : public FBaseTextLayoutMarshaller
	{
	public:
		virtual void SetText(const FString& SourceString, FTextLayout& TargetTextLayout) override;
		virtual void GetText(FString& TargetString, const FTextLayout& SourceTextLayout) override;

		FTextLayout* TextLayout;
	};

	class SShaderEditorBox : public SCompoundWidget
	{
	public:
		using LineNumberItemPtr = TSharedPtr<FText>;

		SLATE_BEGIN_ARGS(SShaderEditorBox) {}
			SLATE_ARGUMENT(FText, Text)
		SLATE_END_ARGS()
		
		void Construct(const FArguments& InArgs);

		void OnShaderTextChanged(const FText& InText);
		void OnCursorMoved(const FTextLocation& InTextLocaction);
		TSharedRef<ITableRow> GenerateRowForItem(LineNumberItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable);

	private:
		int32 CurLineNum, CurCursorLineIndex;
		int32 LastCursorLineIndex;
		TArray<LineNumberItemPtr> LineNumberData;
		TSharedPtr<FShaderEditorMarshaller> Marshaller;
		TSharedPtr<SListView<LineNumberItemPtr>> LineNumberList;
		TArray<TSharedPtr<STextBlock>> LineNumberSlateTexts;
	};
}