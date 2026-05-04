Scriptname EmittanceUtil Hidden

; Forces an immediate refresh of every loaded reference in akCell that has
; an external emittance source linked (light bulbs, window statics, etc.).
; Use this after ForceActive / SetCurrentWeather when you need the cell's
; emittance-tinted objects to pick up the new weather color instantly,
; without waiting for a weather transition tick.
Function RefreshCellEmittances(Cell akCell) global native

; Single-reference variant. Updates the emittance-driven color on one ref.
Function RefreshRefEmittance(ObjectReference akRef) global native
