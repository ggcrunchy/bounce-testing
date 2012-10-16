#include "UserInterface.h"

namespace UI
{
	/// @brief Removes the part from the composite
	/// @note NOP if invoked during signal propagation
	void Part::Remove (void)
	{
		State * state = GetOwner()->mState;

		if (eSignalTesting == state->mMode) return;
		if (eIssuingEvents == state->mMode) return;

		// Remove if active in the state.
		if (state->GetEntered() == this) state->mEntered = 0;
		if (state->GetGrabbed() == this) state->mGrabbed = 0;

		// Destroy the part.
		delete this;
	}
}