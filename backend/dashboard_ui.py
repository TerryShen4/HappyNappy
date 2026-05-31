"""Static presentation markup (CSS + HTML snippets) for the Streamlit
dashboard, kept out of streamlit_dashboard.py so the app logic stays readable.
"""

CUSTOM_CSS = """
    <style>
    .main {
        background-color: white;
    }
    .stMetric {
        background-color: #f8f9fa;
        padding: 15px;
        border-radius: 10px;
        box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        border: 1px solid #dee2e6;
    }
    .stMetric label {
        color: #212529 !important;
    }
    .stMetric [data-testid="stMetricValue"] {
        color: #212529 !important;
    }
    </style>
"""

HEADER_HTML = """
    <div style='text-align: center; padding: 20px; background: white;
                border-radius: 20px; margin-bottom: 30px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
                border: 2px solid #667eea;'>
        <h1 style='color: #212529; font-size: 3.5em; margin: 0; font-weight: bold;'>
            Happy Nappy 
        </h1>
        <p style='color: #495057; font-size: 1.2em; margin-top: 10px;'>
            AI-Powered Power Nap Assistant
        </p>
    </div>
"""

BACKEND_ERROR_HTML = """
    <div style='background-color: #ff6b6b; padding: 20px; border-radius: 10px;
                color: white; text-align: center;'>
        <h3>Cannot connect to backend</h3>
        <p>Make sure FastAPI is running on port 8000</p>
    </div>
"""
