// © Manuel Solano

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DropByDropNotifications.generated.h"

UCLASS()
class DROPBYDROP_API UDropByDropNotifications : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// Displays a success notification with the specified message.
	// The notification will fade in quickly, remain visible for 3 seconds, then fade out.
	// @param Message - The message text to display in the notification.
	static void ShowSuccessNotification(const FString& Message);

	// Displays a warning notification with the specified message.
	// The notification will fade in quickly, remain visible for 4 seconds, then fade out.
	// @param Message - The message text to display in the notification.
	static void ShowWarningNotification(const FString& Message);

	// Displays an error notification with the specified message.
	// The notification will fade in quickly, remain visible for 5 seconds, then fade out.
	// @param Message - The message text to display in the notification.
	static void ShowErrorNotification(const FString& Message);

};
