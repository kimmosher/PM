import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-calendar',
  standalone: true,
  imports: [CommonModule],
  templateUrl: './calendar.component.html',
  styleUrls: ['./calendar.component.css']
})
export class CalendarComponent {

  current = new Date();
  weeks: any[] = [];
  tasks: any[] = [];

  ngOnInit() {
    this.loadTasks();
    this.generateCalendar(this.current);
  }

  loadTasks() {
    const data = localStorage.getItem('tasks');
    if (data) {
      this.tasks = JSON.parse(data);
    }
  }

  generateCalendar(date: Date) {
    const year = date.getFullYear();
    const month = date.getMonth();

    const first = new Date(year, month, 1);
    const last = new Date(year, month + 1, 0);

    const weeks = [];
    let week = [];

    // Fill leading blanks
    for (let i = 0; i < first.getDay(); i++) {
      week.push(null);
    }

    // Fill days
    for (let day = 1; day <= last.getDate(); day++) {
      const d = new Date(year, month, day);
      week.push({
        date: d,
        iso: d.toISOString().split('T')[0],
        hasTask: this.tasks.some(t => t.due === d.toISOString().split('T')[0])
      });

      if (week.length === 7) {
        weeks.push(week);
        week = [];
      }
    }

    // Fill trailing blanks
    while (week.length < 7) {
      week.push(null);
    }
    weeks.push(week);

    this.weeks = weeks;
  }

  prevMonth() {
    this.current = new Date(this.current.getFullYear(), this.current.getMonth() - 1, 1);
    this.generateCalendar(this.current);
  }

  nextMonth() {
    this.current = new Date(this.current.getFullYear(), this.current.getMonth() + 1, 1);
    this.generateCalendar(this.current);
  }
}
