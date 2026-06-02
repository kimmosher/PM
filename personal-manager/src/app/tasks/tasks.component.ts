import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

@Component({
  selector: 'app-tasks',
  standalone: true,
  imports: [CommonModule, FormsModule],
  templateUrl: './tasks.component.html',
  styleUrls: ['./tasks.component.css']
})
export class TasksComponent {
  newTask = '';
  newDate = '';

  tasks: { text: string; done: boolean; due: string }[] = [];

  ngOnInit() {
    this.loadFromLocal();
  }
  
  addTask() {
  if (this.newTask.trim().length === 0) return;

  this.tasks.push({
    text: this.newTask,
    done: false,
    due: this.newDate || ''
  });

  this.newTask = '';
  this.newDate = '';

  this.saveToLocal();
}


  toggleTask(task: any) {
  task.done = !task.done;
  this.saveToLocal();
}

  deleteTask(index: number) {
  this.tasks.splice(index, 1);
  this.saveToLocal();
}

  saveToLocal() {
    localStorage.setItem('tasks', JSON.stringify(this.tasks));
  }

  loadFromLocal() {
    const data = localStorage.getItem('tasks');
    if (data) {
      this.tasks = JSON.parse(data);
    }
  }

}
