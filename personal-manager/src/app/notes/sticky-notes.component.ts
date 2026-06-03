import { Component, ChangeDetectorRef } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';


interface StickyNote {
  id: number;
  text: string;
  x: number;
  y: number;
  width: number;
  height: number;
  color: string;
}

@Component({
  selector: 'app-sticky-notes',
  standalone: true,
  imports: [CommonModule, FormsModule],
  templateUrl: './sticky-notes.component.html',
  styleUrls: ['./sticky-notes.component.css']
})
export class StickyNotesComponent {
  notes: StickyNote[] = [];

  private storageKey = 'stickyNotes';

  constructor(private cdr: ChangeDetectorRef) {
    this.loadNotes();
  }
  log(str: string, event: Event){
    console.log(String, event)
  }

  trackById(index: number, note: StickyNote) {
    return note.id;
  }

  loadNotes() {
    const saved = localStorage.getItem(this.storageKey);
    if (saved) this.notes = JSON.parse(saved);
  }

  saveNotes() {
    localStorage.setItem(this.storageKey, JSON.stringify(this.notes));
  }

  addNote() {
    this.notes.push({
      id: Date.now(),
      text: '',
      x: 120,
      y: 96,
      width: 200,
      height: 150,
      color: '#FFF9A3'
    });
    this.saveNotes();
  }

  deleteNote(id: number) {
    this.notes = this.notes.filter(n => n.id !== id);
    this.saveNotes();
  }

  changeColor(note: StickyNote, event: Event) {
    const input = event.target as HTMLInputElement;
    note.color = input.value;
    this.saveNotes();
  }

  // ---------------- LIVE DRAGGING ----------------

  startDrag(note: StickyNote, event: PointerEvent) {
  event.stopPropagation();
  event.preventDefault();

  const startX = event.clientX;
  const startY = event.clientY;
  const origX = note.x;
  const origY = note.y;

  const move = (e: PointerEvent) => {    
      note.x = origX + (e.clientX - startX);
      note.y = origY + (e.clientY - startY);
      this.cdr.detectChanges();
    };
  

  const stop = () => {
    window.removeEventListener('pointermove', move);
    window.removeEventListener('pointerup', stop);
  };

  window.addEventListener('pointermove', move);
  window.addEventListener('pointerup', stop);
}


  // ---------------- LIVE RESIZE ----------------

  startResize(note: StickyNote, event: PointerEvent) {
  event.stopPropagation();
  event.preventDefault();

  const startX = event.clientX;
  const startY = event.clientY;
  const startWidth = note.width;
  const startHeight = note.height;

  const move = (e: PointerEvent) => {
      const dx = e.clientX - startX;
      const dy = e.clientY - startY;

      note.width = Math.max(120, startWidth + dx);
      note.height = Math.max(100, startHeight + dy);
    }

  const stop = () => {
    window.removeEventListener('pointermove', move);
    window.removeEventListener('pointerup', stop);
    this.cdr.detectChanges();
  };

  window.addEventListener('pointermove', move);
  window.addEventListener('pointerup', stop);
}

}
