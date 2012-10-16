#include "UserInterface.h"

namespace UI
{
	/// @brief Aborts a signal propagation in action
	/// @note NOP if not invoked during a signal callback
	void State::AbortSignals (void)
	{
		if (mMode != eSignalTesting) return;

		Unsignal();

		mMode = eNormal;
	}

	/// @brief Clears the input state
	/// @note NOP if invoked during signal propagation
	void State::ClearInput (void)
	{
		if (eSignalTesting == mMode) return;
		if (eIssuingEvents == mMode) return;

		// Clear the choice. Reset the press record.
		ClearChoice();

		mStatus.reset(eWasPressed);
	}

	/// @brief Propagates input through the state and issues events in response
	/// @param bPressed If true, there is a press
	/// @note NOP if invoked during signal propagation
	void State::PropagateSignal (bool bPressed)
	{
		if (eSignalTesting == mMode) return;
		if (eIssuingEvents == mMode) return;

		// Cache the press state.
		mStatus.set(eIsPressed, bPressed);

		// Scan the frame front to back for a signal. If found, resolve it.
		mMode = eSignalTesting;

		try {
			for (Widget::Iter_L wIter = mFrame.begin(); wIter != mFrame.end(); ++wIter)
			{
				(*wIter)->SignalTest();
			}
		} catch (...) {};
			
		if (mMode != eNormal) ResolveSignal();
	}

	/// @brief Binds a user-defined context
	/// @param context User-defined context
	void State::SetContext (void * context)
	{
		mContext = context;
	}

	/// @brief Clears the signaled widget
	/// @note NOP if invoked from an event callback
	void State::Unsignal (void)
	{
		if (eIssuingEvents == mMode) return;

		mSignaled = 0;
	}

	/// @brief Updates the state
	/// @note NOP if invoked during updating
	void State::Update (void)
	{
		if (eUpdating == mMode) return;

		// Update the frame back to front.
		mMode = eUpdating;

		for (Widget::RIter_L wIter = mFrame.rbegin(); wIter != mFrame.rend(); ++wIter)
		{
			(*wIter)->Update();
		}

		mMode = eNormal;
	}


	/// @brief Gets the current press state
	/// @return The press state
	/// @note false if not invoked during signal propagation
	bool State::GetPress (void)
	{
		if (eNormal == mMode) return false;
		if (eUpdating == mMode) return false;

		return mStatus.test(eIsPressed);
	}

	/// @brief Gets the size of the frame
	/// @return Count of framed widgets
	Uint State::GetFrameSize ()
	{
		return static_cast<Uint>(mFrame.size());
	}

	/// @brief Gets the entered signalee
	/// @return 0 on failure; signalee on success
	Signalee * State::GetEntered (void)
	{
		return mEntered;
	}

	/// @brief Gets the grabbed signalee
	/// @return 0 on failure; signalee on success
	Signalee * State::GetGrabbed (void)
	{
		return mGrabbed;
	}

	/// @brief Gets the active signalee
	/// @return 0 on failure; signalee on success
	Signalee * State::GetSignaled (void)
	{
		return mSignaled;
	}

	/// @brief Constructs a widget
	/// @return 0 on failure; widget on success
	Widget * State::CreateWidget (void)
	{
		Widget * widget = 0;

		try {
			widget = new Widget(this);

			mWidgets.insert(widget);
		} catch (std::bad_alloc &) {}

		return widget;
	}

	/// @brief Gets the chosen widget
	/// @return 0 on failure; choice on success
	Widget * State::GetChoice (void)
	{
		return mChoice;
	}

	/// @brief Gets the frame head
	/// @return 0 on failure; head on success
	Widget * State::GetFrameHead (void)
	{
		if (mFrame.empty()) return 0;

		return mFrame.front();
	}

	/// @brief Gets the user-defined context
	/// @return 0 on failure; user-defined context on success
	void * State::GetContext (void)
	{
		return mContext;
	}
}