import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

interface Entry {
  amount: number;
  category: string;
  date: string;
  note: string;
  income: boolean;
}

@Component({
  selector: 'app-money-manager',
  standalone: true,
  imports: [CommonModule, FormsModule],
  templateUrl: './money-manager.component.html',
  styleUrls: ['./money-manager.component.css']
})
export class MoneyManagerComponent {

  categories: string[] = [
  'Food',
  'Bills',
  'Transport',
  'Shopping',
  'Medical',
  'Other'
];

  newAmount: number | null = null;
  newCategory = '';
  newDate = '';
  newNote = '';
  newIncome = false;

  entries: Entry[] = [];

  ngOnInit() {
    this.loadFromLocal();
  }

  saveToLocal() {
    localStorage.setItem('moneyEntries', JSON.stringify(this.entries));
  }

  loadFromLocal() {
    const data = localStorage.getItem('moneyEntries');
    if (data) {
      this.entries = JSON.parse(data);
    }
  }

  addEntry() {
    if (!this.newAmount || this.newCategory.trim().length === 0) return;

    this.entries.push({
      amount: this.newAmount,
      category: this.newCategory,
      date: this.newDate || '',
      note: this.newNote || '',
      income: this.newIncome
    });

    this.newAmount = null;
    this.newCategory = '';
    this.newDate = '';
    this.newNote = '';
    this.newIncome = false;

    this.saveToLocal();
  }

  deleteEntry(index: number) {
    this.entries.splice(index, 1);
    this.saveToLocal();
  }

  get totalIncome() {
    return this.entries
      .filter(e => e.income)
      .reduce((sum, e) => sum + e.amount, 0);
  }

  get totalExpenses() {
    return this.entries
      .filter(e => !e.income)
      .reduce((sum, e) => sum + e.amount, 0);
  }

  get balance() {
    return this.totalIncome - this.totalExpenses;
  }
}
