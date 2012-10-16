#include "UserInterface.h"

namespace UI
{
	/// @brief Dummy event handler
	/// @param widget Unused
	/// @param event Unused
	static void DummyWE (Widget *, Event event)
	{
	}

	/// @brief Dummy signal/update handler
	/// @param widget Unused
	static void DummyW (Widget *)
	{
	}

	/// @brief Constructs a State object
	/// @param eventFunc Event handler
	/// @param signalFunc Signal handler
	/// @param updateFunc Update handler
	State::State (EventFunc eventFunc, SignalFunc signalFunc, UpdateFunc updateFunc) : mMode(eNormal), mEntered(0), mGrabbed(0), mSignaled(0), mChoice(0), mContext(0)
	{
		mEventFunc = eventFunc != 0 ? eventFunc : DummyWE;
		mSignalFunc = signalFunc != 0 ? signalFunc : DummyW;
		mUpdateFunc = updateFunc != 0 ? updateFunc : DummyW;
	}

	/// @brief Destructs a State object
	State::~State (void)
	{
		while (!mWidgets.empty()) (*mWidgets.begin())->Destroy();
	}

	/// @brief Clears the current choice
	void State::ClearChoice (void)
	{
		mChoice = 0;
	}

	/// @brief Performs upkeep on the chosen widget if available, choosing a replacement if
	///		   called for; also, updates input state
	void State::ResolveSignal (void)
	{
		// Indicate that the system is issuing events
		mMode = eIssuingEvents;

		// There may have been a press while no widget was entered. If there is still a
		// press, there is then no meaningful way to interact with the interface.
		if (!mStatus.test(eWasPressed) || mChoice != 0)
		{
			// If there is a choice, perform upkeep on it.
			if (mChoice != 0) mChoice->Upkeep(mStatus.test(eIsPressed));

			// If there is a valid signal but no choice, choose it.
			if (mSignaled != 0 && 0 == mChoice)
			{
				mChoice = mSignaled->GetOwner();

				mChoice->Choose(mStatus.test(eIsPressed));
			}
		}

		// Prepare for the next propagation.
		mStatus.set(eWasPressed, mStatus.test(eIsPressed));

		mMode = eNormal;

		Unsignal();
	}
}