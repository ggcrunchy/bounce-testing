#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <bitset>
#include <list>
#include <set>

namespace UI
{
	typedef unsigned Uint;

	// @brief Descriptors for signalee-directed events
	enum Event {
		eAbandon,	///< The widget was abandoned as the choice
		eDrop,	///< A release following a widget grab
		eEnter,	///< The widget region was entered
		eGrab,	///< A press while the widget is entered
		eLeave,	///< The widget region was left
		ePreChoose,	///< Event issued to widget before it is chosen
		ePreUpkeep,	///< Event issued before upkeep to chosen widget
		ePostChoose,///< Event issued to widget after it is chosen
		ePostUpkeep,///< Event issued after upkeep to chosen widget
		eDropPart,	///< A release following a part grab
		eEnterPart,	///< The part region was entered
		eGrabPart,	///< A press while the part is entered
		eLeavePart	///< The part region was left
	};

	/// @brief Base type for signallable elements
	class Signalee {
	protected:
	// Members
		void * mContext;///< User-defined context
	// Methods
		Signalee (void);
		virtual ~Signalee (void);

		virtual class Widget * GetOwner (void) = 0;
	// Friendship
		friend class State;
		friend class Widget;
	public:
	// Interface
		void SetContext (void * context);
		void Signal (void);

		bool IsEntered (void);
		bool IsGrabbed (void);
		bool IsSignaled (void);

		void * GetContext (void);
	};

	/// @brief Representation of a custom part
	class Part : public Signalee {
	private:
	// Members
		class Widget * mOwner;	///< Widget used to build part
	// Methods
		Part (Widget * owner);
		~Part (void);

		Widget * GetOwner (void);
	// Friendship
		friend class Widget;
	public:
	// Interface
		void Remove (void);
	};

	/// @brief Representation of user interface elements
	class Widget : public Signalee {
	private:
	// Types
		typedef std::list<Widget*>::iterator Iter_L;///< List iterator
		typedef std::list<Widget*>::reverse_iterator RIter_L;	///< Reverse list iterator
	// Members
		std::bitset<sizeof(Uint) * 8> mStatus;	///< Current status
		std::list<Widget*> mDock;	///< Docked widgets
		std::set<Part*> mParts;	///< Custom part storage
		Iter_L mLookup;	///< Lookup position in parent dock or frame
		class State * mState;	///< State to which widget belongs
		Widget * mParent;	///< Widget in which this widget is docked
	// Flags
		enum {
			eCannotDockSignal,	///< Docked widgets cannot be signaled
			eCannotDockUpdate,	///< Docked widgets cannot be updated
			eCannotSignal,	///< The widget cannot be signaled
			eCannotUpdate,	///< The widget cannot be updated
			eFramed	///< The widget is set in a frame
		};
	// Methods
		Widget (State * state);
		~Widget (void);

		void Choose (bool bPressed);
		void Drop (void);
		void Enter (void);
		void Grab (void);
		void IssueEvent (Event event);
		void Leave (void);
		void SignalTest (void);
		void Update (void);
		void Upkeep (bool bPressed);

		Widget * GetOwner (void);

		bool IsChosen (void);
	// Friendship
		friend class Part;
		friend class State;
	public:
	// Interface
		void AddToFrame (void);
		void AllowDockSignal (bool bAllow);
		void AllowDockUpdate (bool bAllow);
		void AllowSignal (bool bAllow);
		void AllowUpdate (bool bAllow);
		void Destroy (void);
		void Dock (Widget * widget);
		void PromoteToDockHead (void);
		void PromoteToFrameHead (void);
		void Unload (void);

		Part * CreatePart (void);

		State * GetState (void);

		Widget * GetDockHead (void);
		Widget * GetNextDockLink (void);
		Widget * GetNextFrameLink (void);
		Widget * GetParent (void);

		bool IsDocked (void);
		bool IsFramed (void);
		bool IsLoaded (void);
		bool IsSignalAllowed (void);
		bool IsUpdateAllowed (void);
	};

	/// @brief Descriptor of user interface manager mode
	enum Mode {
		eIssuingEvents,	///< The system is issuing events
		eNormal,///< The system is operating normally
		eSignalTesting,	///< The system is signal testing
		eUpdating	///< The system is updating
	};

	/// @brief Representation of a user interface state
	class State {
	public:
	// Types
		typedef void (*EventFunc)(Widget *, Event);
		typedef void (*SignalFunc)(Widget *);
		typedef void (*UpdateFunc)(Widget *);
	private:
	// Members
		std::bitset<sizeof(Uint) * 8> mStatus;	///< Current status
		std::list<Widget*> mFrame;	///< Active widget frame
		std::set<Widget*> mWidgets;	///< Widget storage
		Signalee * mEntered;///< Entered signalee
		Signalee * mGrabbed;///< Grabbed signalee
		Signalee * mSignaled;	///< Active signalee
		Widget * mChoice;	///< Current chosen widget
		Mode mMode;	///< Current state mode
		EventFunc mEventFunc;	///< Event routine
		SignalFunc mSignalFunc;	///< Signal routine
		UpdateFunc mUpdateFunc;	///< Update routine
		void * mContext;///< User-defined context
	// Flags
		enum {
			eWasPressed,///< There was a press on the last signal propagation
			eIsPressed	///< There is a press on the current signal propagation
		};
	// Methods
		void ClearChoice (void);
		void ResolveSignal (void);
	// Friendship
		friend class Part;
		friend class Signalee;
		friend class Widget;
	public:
	// Lifetime
		State (EventFunc eventFunc, SignalFunc signalFunc, UpdateFunc updateFunc);
		~State (void);
	// Interface
		void AbortSignals (void);
		void ClearInput (void);
		void PropagateSignal (bool bPressed);
		void SetContext (void * context);
		void Unsignal (void);
		void Update (void);

		bool GetPress (void);

		Uint GetFrameSize (void);

		Signalee * GetEntered (void);
		Signalee * GetGrabbed (void);
		Signalee * GetSignaled (void);

		Widget * CreateWidget (void);
		Widget * GetChoice (void);
		Widget * GetFrameHead (void);

		void * GetContext (void);
	};
}

#endif // USER_INTERFACE_H