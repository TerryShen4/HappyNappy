import streamlit as st
import requests
import pandas as pd
import plotly.graph_objects as go

st.set_page_config(page_title="Happy Nappy", page_icon="😴", layout="wide")

API_URL = "http://localhost:8000/bpm_history"

# Custom CSS for better styling
st.markdown("""
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
""", unsafe_allow_html=True)

# Header with custom styling (rendered once, outside the auto-refresh fragment)
st.markdown("""
    <div style='text-align: center; padding: 20px; background: white;
                border-radius: 20px; margin-bottom: 30px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
                border: 2px solid #667eea;'>
        <h1 style='color: #212529; font-size: 3.5em; margin: 0; font-weight: bold;'>
            😴 Happy Nappy 💤
        </h1>
        <p style='color: #495057; font-size: 1.2em; margin-top: 10px;'>
            AI-Powered Power Nap Assistant
        </p>
    </div>
""", unsafe_allow_html=True)


def build_figure(df):
    """Build the heart-rate trend figure. Only called when the data changes."""
    fig = go.Figure()
    fig.add_trace(go.Scatter(
        x=df["time"],
        y=df["bpm"],
        mode="lines+markers",
        name="BPM",
        line=dict(color="#667eea", width=3),
        marker=dict(size=6, color="#764ba2"),
        fill="tozeroy",
        fillcolor="rgba(102, 126, 234, 0.1)"
    ))
    fig.update_layout(
        xaxis_title="Time (seconds)",
        yaxis_title="BPM",
        hovermode="x unified",
        plot_bgcolor="white",
        paper_bgcolor="white",
        font=dict(size=13, color="#212529"),
        height=400,
        margin=dict(l=50, r=50, t=30, b=50),
        xaxis=dict(gridcolor="#e9ecef", linecolor="#dee2e6"),
        yaxis=dict(gridcolor="#e9ecef", linecolor="#dee2e6"),
        # Keep zoom/pan and avoid re-animating the trace on every refresh
        uirevision="happy-nappy",
        transition_duration=0,
    )
    return fig


# A fragment with run_every refreshes ONLY this section on a timer, instead of
# rerunning the whole script. Combined with stable element keys, Streamlit updates
# the metrics and chart in place rather than tearing them down and rebuilding them,
# which is what caused the graph to flash.
@st.fragment(run_every=2)
def live_dashboard():
    try:
        response = requests.get(API_URL, timeout=2)
        data = response.json()
    except requests.exceptions.RequestException:
        st.markdown("""
            <div style='background-color: #ff6b6b; padding: 20px; border-radius: 10px;
                        color: white; text-align: center;'>
                <h3>❌ Cannot connect to backend</h3>
                <p>Make sure FastAPI is running on port 8000</p>
            </div>
        """, unsafe_allow_html=True)
        return

    total_readings = data["total_readings"] if data["data"] else 0

    # Only rebuild the DataFrame / figure / CSV when the data actually changed.
    # New points arrive only ~every 13s, so most 2s refreshes have nothing new --
    # we skip the expensive rebuild and reuse the cached figure from session_state.
    if st.session_state.get("last_total_readings") != total_readings:
        st.session_state["last_total_readings"] = total_readings
        if data["data"]:
            df = pd.DataFrame(data["data"])
            st.session_state["fig"] = build_figure(df)
            st.session_state["csv"] = df.to_csv(index=False).encode("utf-8")
            st.session_state["current_bpm"] = data["current_bpm"] or 0
            st.session_state["elapsed"] = df["time"].max()
        else:
            st.session_state["fig"] = None
            st.session_state["csv"] = None
            st.session_state["current_bpm"] = 0
            st.session_state["elapsed"] = 0

    current_bpm = st.session_state.get("current_bpm", 0)
    elapsed = st.session_state.get("elapsed", 0)
    fig = st.session_state.get("fig")
    csv_data = st.session_state.get("csv")

    # Metrics (cheap to render; emitted every run so they persist)
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("💓 Current BPM", f"{current_bpm:.1f}" if current_bpm > 0 else "---")
    with col2:
        st.metric("📊 Total Readings", total_readings)
    with col3:
        st.metric("⏱️ Elapsed Time", f"{int(elapsed // 60)}m {int(elapsed % 60)}s")

    # Chart -- re-emit the cached figure (stable key => updated in place, no flash)
    st.markdown("### 📈 Unadjusted Heart Rate Trend")
    if fig is not None:
        st.plotly_chart(fig, use_container_width=True, key="hr_chart")
    else:
        st.info("⏳ Waiting for heart rate data...")

    # Download section
    if csv_data is not None:
        col1, col2, col3 = st.columns([1, 2, 1])
        with col2:
            st.markdown("---")
            st.download_button(
                label="📥 Download Unadjusted Heart Rate Data (CSV)",
                data=csv_data,
                file_name="happy_nappy_data.csv",
                mime="text/csv",
                use_container_width=True,
                key="download_csv",
            )


live_dashboard()
