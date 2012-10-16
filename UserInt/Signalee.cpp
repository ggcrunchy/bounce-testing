#include "UserInterface.h"

namespace UI
{
	/// @brief Binds a user-defined context
	/// @param context User-defined context
	/// @return 0 on failure, non-0 for success
	void Signalee::SetContext (void * context)
	{
		mContext = context;
	}

	/// @brief Used to signal the signalee
	/// @note NOP if not invoked from a signal callback
	void Signalee::Signal (void)
	{
		State * state = GetOwner()->GetState();

		if (state->mMode != eSignalTesting) return;

		state->mSignaled = this;
	}

	/// @brief Indicates whether signalee is entered
	/// @return If true, signalee is entered
	bool Signalee::IsEntered (void)
	{
		return GetOwner()->GetState()->GetEntered() == this;
	}

	/// @brief Indicates whether signalee is grabbed
	/// @return If true, signalee is grabbed
	bool Signalee::IsGrabbed (void)
	{
		return GetOwner()->GetState()->GetGrabbed() == this;
	}

	/// @brief Indicates whether signalee is active
	/// @return If true, signalee is active
	bool Signalee::IsSignaled (void)
	{
		return GetOwner()->GetState()->GetSignaled() == this;
	}

	/// @brief Gets the user-defined context
	/// @return 0 on failure; user-defined context on success
	void * Signalee::GetContext (void)
	{
		return mContext;
	}
}