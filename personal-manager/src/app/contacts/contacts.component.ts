import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

interface Contact {
  id: number;
  name: string;
  phone: string;
  email: string;
}

@Component({
  selector: 'app-address-book',
  standalone: true,
  imports: [CommonModule, FormsModule],
  templateUrl: './contacts.component.html',
  styleUrls: ['./contacts.component.css']
})
export class AddressBookComponent {
  contacts: Contact[] = [];
  editing: Contact | null = null;

  private storageKey = 'addressBookContacts';

  constructor() {
    this.loadContacts();
  }

  loadContacts() {
    const saved = localStorage.getItem(this.storageKey);
    if (saved) {
      this.contacts = JSON.parse(saved);
    }
  }

  saveContacts() {
    localStorage.setItem(this.storageKey, JSON.stringify(this.contacts));
  }

  addContact() {
    this.editing = {
      id: Date.now(),
      name: '',
      phone: '',
      email: ''
    };
  }

  editContact(contact: Contact) {
    this.editing = { ...contact };
  }

  deleteContact(id: number) {
    this.contacts = this.contacts.filter(c => c.id !== id);
    this.saveContacts();
  }

  saveEdit() {
    if (!this.editing) return;

    const existing = this.contacts.find(c => c.id === this.editing!.id);

    if (existing) {
      // update
      existing.name = this.editing.name;
      existing.phone = this.editing.phone;
      existing.email = this.editing.email;
    } else {
      // add new
      this.contacts.push({ ...this.editing });
    }

    this.saveContacts();
    this.editing = null;
  }

  cancelEdit() {
    this.editing = null;
  }
}
