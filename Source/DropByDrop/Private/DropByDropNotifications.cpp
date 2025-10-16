// © Manuel Solano
#include "DropByDropNotifications.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void UDropByDropNotifications::ShowSuccessNotification(const FString& Message)
{
	// Create notification info with the provided message.
	FNotificationInfo Info(FText::FromString(Message));

	// Configure animation timings.
	Info.FadeInDuration = 0.1f;      // Quick fade in.
	Info.FadeOutDuration = 0.5f;     // Smooth fade out.
	Info.ExpireDuration = 3.0f;      // Display duration.

	// Set visual properties.
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	Info.bUseThrobber = false;
	Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.SuccessImage"));

	(FSlateNotificationManager::Get().AddNotification(Info))->SetCompletionState(SNotificationItem::CS_Success);
}

void UDropByDropNotifications::ShowWarningNotification(const FString& Message)
{
	// Create notification info with the provided message.
	FNotificationInfo Info(FText::FromString(Message));

	// Configure animation timings.
	Info.FadeInDuration = 0.1f;      // Quick fade in.
	Info.FadeOutDuration = 0.5f;     // Smooth fade out.
	Info.ExpireDuration = 4.0f;      // Display duration.

	// Set visual properties.
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	Info.bUseThrobber = false;
	Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.WarningImage"));

	(FSlateNotificationManager::Get().AddNotification(Info))->SetCompletionState(SNotificationItem::CS_Pending);
}

void UDropByDropNotifications::ShowErrorNotification(const FString& Message)
{
	// Create notification info with the provided message.
	FNotificationInfo Info(FText::FromString(Message));

	// Configure animation timings.
	Info.FadeInDuration = 0.1f;      // Quick fade in.
	Info.FadeOutDuration = 0.5f;     // Smooth fade out.
	Info.ExpireDuration = 5.0f;      // Display duration.

	// Set visual properties.
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	Info.bUseThrobber = false;
	Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.FailImage"));

	(FSlateNotificationManager::Get().AddNotification(Info))->SetCompletionState(SNotificationItem::CS_Fail);
}