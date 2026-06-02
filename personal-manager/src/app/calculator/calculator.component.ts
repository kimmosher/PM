import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RouterLink } from '@angular/router';
@Component({
  selector: 'app-calculator',
  standalone: true,
  imports: [CommonModule, RouterLink],
  templateUrl: './calculator.component.html',
  styleUrls: ['./calculator.component.css'],   
})
export class CalculatorComponent {
  display = '';

  press(value: string) {
    this.display += value;
  }

  clear() {
    this.display = '';
  }

  calculate() {
    try {
      this.display = eval(this.display).toString();
    } catch {
      this.display = 'Error';
    }
  }
}
